/* pngz.h
 * samantha jane
 * simple png
 *--------------------------------------------------------------------------80*/

#ifndef _PNGZ_H_
#define _PNGZ_H_

#include <png.h>
/**
 * pixel in RGBA8 format.
 */
typedef struct pixel {
  /* value */
  unsigned char r, g, b, a;
  /* coordinates */
  union {
    unsigned height;
    unsigned row;
  };
  union {
    unsigned width;
    unsigned col;
  };
} pixel;

/**
 * simplified png structure.
 * always contains RGBA8 color.
 * width and height are in pixels.
 * pixel buffer is [height][width] ([rows][cols])
 */
typedef struct pngz {
  char* path;
  union {
    unsigned height;
    unsigned rows;
  };
  union {
    unsigned width;
    unsigned cols;
  };
  pixel** pixels;
} pngz;

/* prototypes */

/* allocate & free */
pixel** pngz_alloc_pixels(unsigned rows, unsigned cols);
unsigned char** pngz_alloc_bytes(unsigned rows, unsigned cols);
int pngz_free_pixels(pixel** pixels, unsigned rows);
int pngz_free_bytes(unsigned char** bytes, unsigned rows);
int pngz_free(pngz* z);

/* load and save */
int pngz_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
);
int pngz_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
);
int pngz_load(pngz* z);
int pngz_load_from(pngz* z, char* path);
int pngz_save(pngz z);
int pngz_save_as(pngz z, char* path);

/* print */
void pngz_print(pngz z);
void pngz_print_pixel(pixel p);
void pngz_print_indent(pngz z, int indent);
void pngz_print_pixel_indent(pixel p, int indent);

#endif
