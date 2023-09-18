/* pngz.c
 * samantha jane
 * easy png library to handle png io
 *
 * normal use case:
 *
 * // load in an image and resave it elsewhere
 * pngz z = {.path = "./img.png"};
 * pngz_load(&z");
 * pngz_save(&z");
 * pngz_save_as(z, "./TEST.png");
 * pngz_free(&z);
 *
 * sources:
 * http://www.libpng.org/pub/png/libpng-manual.txt
 * http://zarb.org/~gc/html/libpng.html
 * https://gist.github.com/niw/5963798
 *--------------------------------------------------------------------------80*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>
#include "pngz.h"


/* alloc & free *-------------------------------------------------------------*/

/**
 * allocate a pixel buffer.
 * every pixel will be represented as a struct
 *
 * @param rows number of rows to allocate
 * @param cols number pixels per col to allocate
 * @return exit code, setting errno on failure
 */
pixel** pngz_alloc_pixels(unsigned rows, unsigned cols) {
  pixel** pixels;
  errno = 0;
  /* allocate rows to hold addresses */
  pixels = malloc(sizeof(void *) * rows);
  if (!pixels) {
    fprintf(stderr, "ERROR > allocing pixel buffer rows.\n");
    errno = ENOMEM;
    return NULL;
  }
  /* allocate each row to hold pixels */
  for (unsigned i = 0; i < rows; i++) {
    pixels[i] = malloc(sizeof(pixel) * cols);
    if (!pixels[i]) {
      fprintf(stderr, "ERROR > allocing pixel buffer cols.\n");
      errno = ENOMEM;
      return NULL;
    }
    /* number pixels for later reference */
    for (unsigned j = 0; j < cols; j++) {
      pixels[i][j].row = i;
      pixels[i][j].col = j;
    }
  }
  return pixels;
}

/**
 * allocate raw pixel byte buffer.
 *
 * @param rows number of rows to allocate
 * @param cols number bytes per row to allocate
 * @return a ptr to buffer, returns null and sets errno on failure
 */
unsigned char** pngz_alloc_bytes(unsigned rows, unsigned cols) {
  errno = 0;
  /* allocate rows to hold addresses */
  unsigned char** bytes = malloc(sizeof(void *) * rows);
  if (!bytes) {
    fprintf(stderr, "ERROR > allocing raw buffer rows.\n");
    errno = ENOMEM;
    return NULL;
  }
  for (unsigned i = 0; i < rows; i++) {
    bytes[i] = malloc(sizeof(unsigned char) * cols);
    if (!bytes[i]) {
      fprintf(stderr, "ERROR > allocing raw buffer cols.\n");
      errno = ENOMEM;
      return NULL;
    }
  }
  return bytes;
}

/**
 * safely free pixel buffer.
 *
 * @param pixels 2D pixel ptr of size [rows][cols]
 * @param rows number of rows to free
 * @return exit code
 */
int pngz_free_pixels(pixel** pixels, unsigned rows) {
  errno = 0;
  int exit_code = 0;
  for (unsigned i = 0; i < rows; i++) {
    if (pixels[i]) {
      free(pixels[i]);
      pixels[i] = NULL;
    } else {
      fprintf(stderr, "ERROR > freeing pixel buffer cols.\n");
      errno = EPERM;
      exit_code = 1;
    }
  }
  if (pixels) {
    free(pixels);
    pixels = NULL;
  } else {
    fprintf(stderr, "ERROR > freeing pixel buffer rows.\n");
    errno = EPERM;
    exit_code = 1;
  }
  return exit_code;
}

/**
 * safely free pixel buffer.
 *
 * @param bytes 2D pixel bytes of size [rows][cols * 4]
 * @param rows number of rows to free
 * @return exit code
 */
int pngz_free_bytes(unsigned char** bytes, unsigned rows) {
  int exit_code;
  errno = 0;
  exit_code = 0;
  for (unsigned i = 0; i < rows; i++) {
    if (bytes[i]) {
      free(bytes[i]);
      bytes[i] = NULL;
    } else {
      fprintf(stderr, "ERROR > freeing raw buffer cols.\n");
      errno = EPERM;
      exit_code = 1;
    }
  }
  if (bytes) {
    free(bytes);
    bytes = NULL;
  } else {
    fprintf(stderr, "ERROR > freeing raw buffer rows.\n");
    errno = EPERM;
    exit_code = 1;
  }
  return exit_code;
}

/**
 * free a pngz ztructs pixels.
 *
 * @param img png* loaded into memory
 * @return exit code, error if you try to free a NULL ptr
 */
int pngz_free(pngz* z) {
  return pngz_free_pixels(z->pixels, z->height);
}

/* loading and saving *-------------------------------------------------------*/

/**
 * pack pixels with bytes.
 *
 * @param bytes_src unpacked pixel byte source
 * @param pixels_dest packed pixel destination
 * @param rows rows of pixels
 * @param cols cols of pixels (IN PIXELS NOT BYTES)
 * @return exit code
 **/
int pngz_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
) {
  for (unsigned r = 0; r < rows; r++) {
    for (unsigned c = 0; c < cols; c++) {
      pixel p = {
        .r = bytes_src[r][c * 4],
        .g = bytes_src[r][(c * 4) + 1],
        .b = bytes_src[r][(c * 4) + 2],
        .a = bytes_src[r][(c * 4) + 3]
      };
      pixels_dest[r][c] = p;
    }
  }
  return 0;
}

/**
 * unpack pixels into raw byte ptrs.
 *
 * @param bytes_src unpacked pixel byte source
 * @param pixels_dest packed pixel destination
 * @param rows rows of pixels
 * @param cols cols of pixels (IN PIXELS NOT BYTES)
 * @return exit code
 **/
int pngz_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
) {
  for (unsigned r = 0; r < rows; r++) {
    for (unsigned c = 0; c < cols; c++) {
      bytes_dest[r][c * 4] = pixels_src[r][c].r;
      bytes_dest[r][(c * 4) + 1] = pixels_src[r][c].g;
      bytes_dest[r][(c * 4) + 2] = pixels_src[r][c].b;
      bytes_dest[r][(c * 4) + 3] = pixels_src[r][c].a;
    }
  }
  return 0;
}

/**
 * load a pngz object into memory
 *
 *
 * @param z pngz* simple png ptr to load into
 * @return exit code
 */
// pngz pngz_load_from_path(char* path) {
int pngz_load(pngz* z) {

  errno = 0;

  /* defaults */
  z->path = z->path ? z->path : "default.png";

  fprintf(stderr, "pngz loading png at '%s'\n", z->path);


  /* open the file and read into info struct */
  FILE *fp;
  if(!(fp = fopen(z->path, "rb"))) {
    fprintf(stderr, "ERROR > opening file.\n");
    errno = EIO;
    return 1;
  }
  png_structp png;
  if (!(png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    fprintf(stderr, "ERROR > creating read struct.\n");
    errno = EIO;
    return 1;
  }
  png_infop info;
  if (!(info = png_create_info_struct(png))) {
    fprintf(stderr, "ERROR > creating png info.\n");
    errno = EIO;
    return 1;
  }
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "ERROR > io failed.\n");
    errno = EIO;
    return 1;
  }

  /* read in png info */
  png_init_io(png, fp);
  png_read_info(png, info);
  unsigned width = png_get_image_width(png, info);
  unsigned height = png_get_image_height(png, info);
  unsigned char bit_depth = png_get_bit_depth(png, info);
  unsigned char color_type = png_get_color_type(png, info);

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

  /* malloc pixel buffers */
  unsigned char** byte_ptrs = pngz_alloc_bytes(height, width * 4);
  z->pixels = pngz_alloc_pixels(height, width);
  /* load into byte buffer, transfer to pixels */
  png_read_image(png, byte_ptrs);
  pngz_pack_pixels(byte_ptrs, z->pixels, height, width);
  /* clean up */
  pngz_free_bytes(byte_ptrs, height);
  png_destroy_read_struct(&png, &info, NULL);
  fclose(fp);

  /* if we got here, pack referenced struct and return */
  z->width  = width;
  z->height = height;
  return 0;
}

/**
 * write a png back out to file
 *
 * @param z pngz* simple png ptr to write to file
 * @return exit code
 */
int pngz_save(pngz z) {
  return pngz_save_as(z, z.path);
}

/**
 * write a png back out to file with a new name
 *
 * @param path char* file path to save to
 * @param z pngz* simple png ptr to write to file
 * @return exit code
 */
int pngz_save_as(pngz z, char* path) {

  FILE *fp;
  png_structp png;
  png_infop info;

  errno = 0;

  /* open the file and write info struct */
  if (!(fp = fopen(path, "wb"))) {
    fprintf(stderr, "ERROR > opening file.\n");
    errno = EIO;
    return 1;
  }

  if (!(png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    fprintf(stderr, "ERROR > creating write struct.\n");
    errno = EIO;
    return 1;
  }
  if (!(info = png_create_info_struct(png))) {
    fprintf(stderr, "ERROR > creating info struct.\n");
    errno = EIO;
    return 1;
  }
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "ERROR > io failed.\n");
    errno = EIO;
    return 1;
  }

  /* write to RGBA8 */
  png_init_io(png, fp);
  png_set_IHDR(
    png,
    info,
    z.width, z.height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);
  unsigned char** bytes = pngz_alloc_bytes(z.height, z.width * 4);
  pngz_unpack_pixels(z.pixels, bytes, z.height, z.width);
  png_write_image(png, bytes);
  png_write_end(png, NULL);

  /* clean up */
  pngz_free_bytes(bytes, z.height);
  png_destroy_write_struct(&png, &info);
  fclose(fp);
  return 0;
}

/* printing functions *-------------------------------------------------------*/

/**
 * print a pngz ztruct contents.
 *
 * @param p pngz to print
 * @return void
 */
void pngz_print(pngz z) {
  printf("PNGZ [\n");
  printf("  rows x cols : %d x %d\n", z.rows, z.cols);
  if (z.pixels) {
    printf("  pixels : loaded\n");
  } else {
    printf("  pixels : empty\n");
  }
  printf("]\n");
  return;
}

/**
 * print out a single pixels rgba values in hex.
 *
 * @param p unsigned char* to the pixel to print
 * @return void
 *
 */
void pngz_print_pixel(pixel p) {
  printf(
    "PIXEL [ row x col: *%u, %u), #%02X%02X%02X%02X ]\n",
    p.row, p.col, p.r, p.g, p.g, p.a
  );
  return;
}

/* private util function to indent */
void print_indent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");
  }
  return;
}

/**
 * print a pngz ztruct contents.
 *
 * @param p pngz to print
 * @return void
 */
void pngz_print_indent(pngz z, int indent) {
  print_indent(indent);
  printf("PNGZ [\n");
  print_indent(indent);
  printf("  row x col : %d x %d\n", z.height, z.width);
  print_indent(indent);
  if (z.pixels) {
    printf("  pixels : loaded\n");
  } else {
    printf("  pixels : empty\n");
  }
  print_indent(indent);
  printf("]\n");
  return;
}

/**
 * print out a single pixels rgba values.
 *
 * @param p unsigned char* to the pixel to print
 * @return void
 *
 */
void pngz_print_pixel_indent(pixel p, int indent) {
  print_indent(indent);
  printf(
    "PIXEL [ #%02X%02X%02X%02X ]\n",
    p.r, p.g, p.g, p.a
  );
  return;
}
