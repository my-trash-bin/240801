#include "bmp.h"

#include <stdlib.h>

static uint32_t u32_to_le(uint32_t u32) {
  const uint32_t test = 42;
  const char *const source = (const char *)&u32;
  uint32_t result;
  char *const dest = (char *)&result;

  if (*((char *)&test))
    return (u32);
  dest[0] = source[3];
  dest[1] = source[2];
  dest[2] = source[1];
  dest[3] = source[0];
  return (result);
}

static uint16_t u16_to_le(uint16_t u16) {
  const uint16_t test = 42;
  const char *const source = (const char *)&u16;
  uint16_t result;
  char *const dest = (char *)&result;

  if (*((char *)&test))
    return (u16);
  dest[0] = source[1];
  dest[1] = source[0];
  return (result);
}

bool serialize_bmp(bmp_t *self, char **out, size_t *out_length) {
  const size_t row_padding = (4 - (self->width * 3) % 4) % 4;
  const size_t row_size = self->width * 3 + row_padding;
  const size_t whole_size = row_size * self->height;
  const size_t length = 54 + whole_size;

  char *const result = malloc(length);
  if (!result)
    return (true);

  // header
  result[0] = 'B';
  result[1] = 'M';
  *((uint32_t *)(result + 2)) = u32_to_le(54 + (uint32_t)whole_size);
  *((uint32_t *)(result + 6)) = u32_to_le(0);
  *((uint32_t *)(result + 10)) = u32_to_le(54);
  *((uint32_t *)(result + 14)) = u32_to_le(40);
  *((uint32_t *)(result + 18)) = u32_to_le((uint32_t)self->width);
  *((uint32_t *)(result + 22)) = u32_to_le((uint32_t)self->height);
  *((uint16_t *)(result + 26)) = u16_to_le(1);
  *((uint16_t *)(result + 28)) = u16_to_le(24);
  *((uint32_t *)(result + 30)) = u32_to_le(0);
  *((uint32_t *)(result + 34)) = u32_to_le((uint32_t)whole_size);
  *((uint32_t *)(result + 38)) = u32_to_le(0);
  *((uint32_t *)(result + 42)) = u32_to_le(0);
  *((uint32_t *)(result + 46)) = u32_to_le(0);
  *((uint32_t *)(result + 50)) = u32_to_le(0);

  // image data
  size_t offset = 54;
  for (size_t y = self->height - 1; y != (size_t)-1; y--) { // per line
    for (size_t x = 0; x < self->width; x++) {
      *((uint8_t *)&result[offset++]) = self->extra[self->width * y + x].b;
      *((uint8_t *)&result[offset++]) = self->extra[self->width * y + x].g;
      *((uint8_t *)&result[offset++]) = self->extra[self->width * y + x].r;
    }
    // padding
    for (size_t i = 0; i < row_padding; i++)
      result[offset++] = 0;
  }

  *out_length = length;
  *out = result;
  return (false);
}
