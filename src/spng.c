/* spng.c
 * samantha jane
 * simple png library to handle loading, saving, mapping, and printing
 * sources:
 * http://www.libpng.org/pub/png/libpng-manual.txt
 * http://zarb.org/~gc/html/libpng.html
 * https://gist.github.com/niw/5963798
 *--------------------------------------------------------------------------80*/

#include <png.h>
#include <stdlib.h>
#include "spng.h"

/**
 * load a png image as a spng struct as  RGBA8.
 *
 * @param path char* file path to load
 * @param p spng* simple png ptr to load into
 * @return exit code
 */
int spng_load(char* path, spng* s) {

  unsigned int i;
  FILE *fp;
  png_structp png;
  png_infop info;
  unsigned int width, height;
  unsigned char bit_depth, color_type;
  unsigned char** row_ptrs;

  fprintf(stderr, "loading png %s\n", path);

  /* open the file */
  fp = fopen(path, "rb");
  if(!fp) {
    fprintf(stderr, "ERROR > opening file.\n");
    return 1;
  }

  /* create a read and info struct */
  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) {
    fprintf(stderr, "ERROR > creating read struct.\n");
    return 1;
  }
  info = png_create_info_struct(png);
  if(!info) {
    fprintf(stderr, "ERROR > creating png info.\n");
    return 1;
  }
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "ERROR > io failed.\n");
    return 1;
  }

  /* read in png info */
  png_init_io(png, fp);
  png_read_info(png, info);
  width = png_get_image_width(png, info);
  height = png_get_image_height(png, info);
  bit_depth = png_get_bit_depth(png, info);
  color_type = png_get_color_type(png, info);

  /* convert contents into RGBA8 */
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }
  if (bit_depth == 16) {
    png_set_strip_16(png);
  }
  if (bit_depth < 8) {
    if (color_type == PNG_COLOR_TYPE_GRAY) {
      png_set_expand_gray_1_2_4_to_8(png);
    } else {
      png_set_expand(png);
    }
  }
  if (png_get_valid(png, info, PNG_INFO_tRNS)){
    png_set_tRNS_to_alpha(png);
  }
  if (
    color_type != PNG_COLOR_TYPE_RGBA ||
    color_type != PNG_COLOR_TYPE_GRAY_ALPHA
  ) {
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  }
  if (
    color_type != PNG_COLOR_TYPE_GRAY ||
    color_type != PNG_COLOR_TYPE_GRAY_ALPHA
  ) {
    png_set_gray_to_rgb(png);
  }
  png_read_update_info(png, info);

  /* malloc row ptrs to load it into */
  row_ptrs = malloc(sizeof(unsigned char*) * height);
  if (!row_ptrs) {
    fprintf(stderr, "ERROR > couldn't malloc rows.\n");
    return 1;
  }
  for (i = 0; i < height; i++) {
    row_ptrs[i] = malloc(png_get_rowbytes(png, info));
    if (!row_ptrs[i]) {
      fprintf(stderr, "ERROR > couldn't malloc a row.\n");
      return 1;
    }
  }
  /* load into pixel buffer */
  png_read_image(png, row_ptrs);

  /* clean up */
  png_destroy_read_struct(&png, &info, NULL);
  fclose(fp);

  /* if we got here, pack referenced struct and return */
  s->width  = width;
  s->height = height;
  s->pixels = row_ptrs;
  return 0;
}

/**
 * free a spng structs pixels.
 *
 * @param img png* loaded into memory
 * @return exit code, error if you try to free a NULL ptr
 */
int spng_free(spng* s) {
  int exit_code = 0;
  unsigned int i;
  if (s->pixels) {
    /* free all rows */
    for (i = 0; i < s->height; i++) {
      if (s->pixels[i]) {
        free(s->pixels[i]);
      } else {
        exit_code = 1;
      }
    }
    free(s->pixels);
  } else {
    exit_code = 1;
  }
  return exit_code;
}

/**
 * write a png back out to file
 *
 * @param path char* file path to save to
 * @param p spng* simple png ptr to write to file
 * @return exit code
 */
int spng_write(char* path, spng* s) {

  FILE *fp;
  png_structp png;
  png_infop info;

  /* open the file */
  fp = fopen(path, "wb");
  if(!fp) {
    fprintf(stderr, "ERROR > opening file.\n");
    return 1;
  }

  /* create write and info struct */
  png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) {
    fprintf(stderr, "ERROR > creating write struct.\n");
    return 1;
  }
  info = png_create_info_struct(png);
  if (!info) {
    fprintf(stderr, "ERROR > creating info struct.\n");
    return 1;
  }
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "ERROR > io failed.\n");
    return 1;
  }

  /* write to RGBA8 */
  png_init_io(png, fp);
  png_set_IHDR(
    png,
    info,
    s->width, s->height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);
  png_write_image(png, s->pixels);
  png_write_end(png, NULL);

  /* clean up */
  png_destroy_write_struct(&png, &info);
  spng_free(s);
  fclose(fp);
  return 0;
}

/**
 * simple png filter.
 * maps a function onto every pixel.
 *
 * @param p spng* simple png to filter
 * @param filter void(*)(unsigned char*), a filter function that take pixel ptr
 * @return exit code
 */
int spng_filter(spng* s, void(*filter)(unsigned char*)) {
  unsigned int row, col;
  for (row = 0; row < s->height; row++) {
    for (col = 0; col < s->width; col++) {
      filter(&(s->pixels[row][col]));
    }
  }
  return 0;
}

/**
 * print a spng struct contents.
 *
 * @param p spng to print
 * @return void
 */
void spng_print(spng s) {
  printf("SPNG [\n");
  printf("  row x col : %d, %d\n", s.width, s.height);
  if (s.pixels) {
    printf("  pixels : loaded\n");
  } else {
    printf("  pixels : empty\n");
  }
  printf("]\n");
}

/**
 * print out a single pixels rgba values.
 *
 * @param p unsigned char* to the pixel to print
 * @return void
 *
 */
void spng_print_rgba(unsigned char* p) {
  printf(
   "[ %d, %d, %d, %d ]\n",
   p[0], p[1], p[2], p[3]
  );
}
