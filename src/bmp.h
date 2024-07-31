#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct bmp_pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} bmp_pixel_t;

typedef struct bmp {
  size_t width;
  size_t height;
  bmp_pixel_t extra[];
} bmp_t;

/**
 * @brief Serialize bmp
 *
 * @param self target image
 * @param out serialized result
 * @param out_length result length
 * @return true on failure
 * @return false on success
 */
bool serialize_bmp(bmp_t *self, char **out, size_t *out_length);
