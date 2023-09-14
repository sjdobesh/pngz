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
typedef struct spng {
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
} spng;

/**
 * thread parameters for multithreaded image filtering
 */
typedef struct thread_parameters {
  spng* i_image; /* reference to the image */
  pixel** o_buf; /* reference to the output */
  pixel (*filter_function) (pixel); /* filter function to apply */
  unsigned long row_start, row_stop;
  int thread_id;
} thread_parameters;

/* prototypes */

/* allocate & free */
pixel** spng_alloc_pixels(unsigned rows, unsigned cols);
unsigned char** spng_alloc_bytes(unsigned rows, unsigned cols);
int spng_free_pixels(pixel** pixels, unsigned rows);
int spng_free_bytes(unsigned char** bytes, unsigned rows);
int spng_free(spng* s);

/* load and save */
int spng_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
);
int spng_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
);
int spng_load(spng* s);
int spng_save(spng s);
int spng_save_as(spng s, char* path);

/* filter functions */
void* spng_filter_threadfn(void* params);
int spng_filter_threaded(spng* s, pixel(*filter)(pixel), unsigned thread_count);
int spng_filter(spng* s, pixel(*filter)(pixel));

/* print */
void spng_print(spng s);
void spng_print_pixel(pixel p);
void spng_print_indent(spng s, int indent);
void spng_print_pixel_indent(pixel p, int indent);

#endif
