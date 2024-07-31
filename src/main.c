#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bmp.h"

static const GLfloat vertices[] = {
    0.0f,  0.5f, 0.0f,
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};

static const char* vertex_shader_source =
    "attribute vec4 position;\n"
    "void main() {\n"
    "    gl_Position = position;\n"
    "}\n";

static const char* fragment_shader_source =
    "void main() {\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

GLuint load_shader(GLenum type, const char *shader_src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_src, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char *info_log = malloc(info_len);
            glGetShaderInfoLog(shader, info_len, NULL, info_log);
            fprintf(stderr, "Error compiling shader: %s\n", info_log);
            free(info_log);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int main() {
    // EGL setup
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    EGLint num_config;
    EGLint attr[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE};

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, NULL, NULL);
    eglChooseConfig(display, attr, &config, 1, &num_config);
    EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);

    // Create a pbuffer surface
    EGLint pbuffer_attr[] = {
        EGL_WIDTH, 640,
        EGL_HEIGHT, 480,
        EGL_NONE,
    };
    surface = eglCreatePbufferSurface(display, config, pbuffer_attr);
    eglMakeCurrent(display, surface, surface, context);

    // Set up OpenGL ES
    GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);

    GLuint position_loc = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    glViewport(0, 0, 640, 480);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Read pixels
    unsigned char *pixels = malloc(640 * 480 * 4);
    glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Convert to bmp structure
    bmp_t *bmp_image = malloc(sizeof(bmp_t) + 640 * 480 * sizeof(bmp_pixel_t));
    bmp_image->width = 640;
    bmp_image->height = 480;
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            int index = (y * 640 + x) * 4;
            bmp_image->extra[y * 640 + x].r = pixels[index];
            bmp_image->extra[y * 640 + x].g = pixels[index + 1];
            bmp_image->extra[y * 640 + x].b = pixels[index + 2];
        }
    }
    free(pixels);

    // Serialize to BMP
    char *bmp_data;
    size_t bmp_size;
    if (serialize_bmp(bmp_image, &bmp_data, &bmp_size)) {
        fprintf(stderr, "Failed to serialize BMP\n");
    } else {
        FILE *f = fopen("output.bmp", "wb");
        fwrite(bmp_data, 1, bmp_size, f);
        fclose(f);
        free(bmp_data);
    }
    free(bmp_image);

    // Clean up
    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);

    return 0;
}
