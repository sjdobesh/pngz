/* spng.h
 * samantha jane
 * simple png
 *--------------------------------------------------------------------------80*/

#ifndef _SPNG_H_
#define _SPNG_H_

#include <png.h>

/**
 * pixel in RGBA8 format.
 */
typedef struct pixel {
  unsigned char r, g, b, a;
} pixel;

/**
 * simplified png structure.
 * always contains RGBA8 color.
 * width and height are in pixels.
 * pixel buffer is [height][width] ([row][col])
 */
typedef struct spng {
  unsigned int width;
  unsigned int height;
  pixel** pixels;
} spng;

/* prototypes */

/* allocate & free */
pixel** spng_alloc_pixels(unsigned int rows, unsigned int cols);
unsigned char** spng_alloc_bytes(unsigned int rows, unsigned int cols);
int spng_free_pixels(pixel** pixels, unsigned int rows);
int spng_free_bytes(unsigned char** bytes, unsigned int rows);
int spng_free(spng* p);

/* load and save */
int spng_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned int rows, unsigned int cols
);
int spng_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned int rows, unsigned int cols
);
int spng_load(spng* p, char* path);
int spng_save(spng p, char* path);

/* utility functions */
int spng_filter(spng* p, void(*filter)(pixel*));

/* print */
void spng_print(spng p);
void spng_print_pixel(pixel p);
void spng_print_indent(spng p, int indent);
void spng_print_pixel_indent(pixel p, int indent);

#endif
