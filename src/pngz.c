/* pngz.c
 * samantha jane
 * easy png library to handle png io
 *
 * normal use case:
 *
 * // load in an image and resave it elsewhere
 * PNGZ_Image z = {.path = "./img.png"};
 * PNGZ_Load(&z");
 * PNGZ_Save(&z");
 * PNGZ_SaveAs(z, "./TEST.png");
 * PNGZ_Free(&z);
 *
 * sources for libpng :
 * http://www.libpng.org/pub/png/libpng-manual.txt
 * http://zarb.org/~gc/html/libpng.html
 * https://gist.github.com/niw/5963798
 *--------------------------------------------------------------------------80*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
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
int PNGZ_AllocPixels(PNGZ_Pixel*** pixels_ptr, const unsigned rows, const unsigned cols) {
  unsigned i;
  errno = 0;
  /* allocate row ptrs (Y) */
  if ( ! (*pixels_ptr = malloc(sizeof(void *) * rows))) {
    fprintf(stderr, "ERROR > allocing pixel buffer rows.\n");
    errno = ENOMEM;
    return 1;
  }
  /* allocate each row with a column number of pixels (X) */
  for (i = 0; i < rows; i++) {
    if ( ! ((*pixels_ptr)[i] = malloc(sizeof(PNGZ_Pixel) * cols))) {
      fprintf(stderr, "ERROR > allocing pixel buffer cols.\n");
      errno = ENOMEM;
      /* if error, undo any allocations that were made */
      free (*pixels_ptr);
      for (; i > 0; i--) {
        free ((*pixels_ptr)[i]);
      }
      return 1;
    }
  }
  return 0;
}

/**
 * allocate raw pixel byte buffer.
 *
 * @param rows number of rows to allocate
 * @param cols number bytes per row to allocate
 * @return a ptr to buffer, returns null and sets errno on failure
 */
int PNGZ_AllocBytes(uint8_t*** bytes_ptr, const unsigned rows, const unsigned cols) {
  unsigned i;
  errno = 0;
  /* allocate row ptrs (Y) */
  if ( ! (*bytes_ptr = malloc(sizeof(void *) * rows))) {
    fprintf(stderr, "ERROR > allocing byte buffer rows.\n");
    errno = ENOMEM;
    return 1;
  }
  /* allocate each row with a column number of bytes (X*4) */
  for (i = 0; i < rows; i++) {
    if ( ! ((*bytes_ptr)[i] = malloc(sizeof(uint8_t) * cols))) {
      fprintf(stderr, "ERROR > allocing byte buffer cols.\n");
      errno = ENOMEM;
      /* if error, undo any allocations that were made */
      free (*bytes_ptr);
      for (; i > 0; i--) {
        free ((*bytes_ptr)[i]);
      }
      return 1;
    }
  }
  return 0;
}

/**
 * safely free pixel buffer.
 *
 * @param pixels 2D pixel ptr of size [rows][cols]
 * @param rows number of rows to free
 * @return exit code
 */
int PNGZ_FreePixels(PNGZ_Pixel** pixels, const unsigned rows) {
  unsigned i;
  int exit_code = 0;
  errno = 0;
  /* deallocate column ptrs (X) */
  for (i = 0; i < rows; i++) {
    if (pixels[i]) {
      free(pixels[i]);
      pixels[i] = NULL;
    } else {
      fprintf(stderr, "ERROR > freeing pixel buffer cols.\n");
      errno = EPERM;
      /* no early return, keep trying to free other ptrs */
      exit_code = 1;
    }
  }
  /* deallocate row ptrs (Y) */
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
int PNGZ_FreeBytes(uint8_t ** bytes, const unsigned rows) {
  unsigned i;
  int exit_code = 0;
  errno = 0;
  /* deallocate column ptrs (X) */
  for (i = 0; i < rows; i++) {
    if (bytes[i]) {
      free(bytes[i]);
      bytes[i] = NULL;
    } else {
      fprintf(stderr, "ERROR > freeing raw buffer cols.\n");
      errno = EPERM;
      exit_code = 1;
    }
  }
  /* deallocate row ptrs (Y) */
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
 * free a pngz structs pixels.
 *
 * @param z pngz* with pixels loaded into memory
 * @return exit code, error if you try to free a NULL ptr
 */
int PNGZ_Free(PNGZ_Image* z) {
  return PNGZ_FreePixels(z->pixels, z->height);
}

/* loading and saving *-------------------------------------------------------*/

/**
 * pack bytes into pixels.
 *
 * @param bytes_src unpacked pixel byte source
 * @param pixels_dest packed pixel destination
 * @param rows rows of pixels (Y)
 * @param cols cols of pixels (X) (IN PIXELS NOT BYTES)
 * @return exit code
 **/
int PNGZ_BytesToPixels(
  uint8_t** bytes_src, PNGZ_Pixel** pixels_dest,
  const unsigned rows, const unsigned cols
) {
  unsigned r, c;
  PNGZ_Pixel p;
  errno = 0;
  if (!bytes_src) {
    fprintf(stderr, "ERROR > unpacking pixels from null source.\n");
    errno = EPERM;
    return 1;
  }
  if (!pixels_dest) {
    fprintf(stderr, "ERROR > unpacking pixels to null destination.\n");
    errno = EPERM;
    return 1;
  }
  for (r = 0; r < rows; r++)
    for (c = 0; c < cols; c++) {
      p.r = bytes_src[r][c * 4];
      p.g = bytes_src[r][(c * 4) + 1];
      p.b = bytes_src[r][(c * 4) + 2];
      p.a = bytes_src[r][(c * 4) + 3];
      pixels_dest[r][c] = p;
    }
  return 0;
}

/**
 * unpack pixels into raw byte ptrs.
 * expects adequate space, rows by cols
 * in the pixel buffer and rows by cols * 4
 * in the byte buffer
 *
 * @param pixels_src packed pixel source
 * @param bytes_dest unpacked byte destination
 * @param rows rows of pixels
 * @param cols cols of pixels (IN PIXELS NOT BYTES)
 * @return exit code
 **/
int PNGZ_PixelsToBytes(
  PNGZ_Pixel** pixels_src, uint8_t** bytes_dest,
  const unsigned rows, const unsigned cols
) {
  unsigned r, c;
  errno = 0;
  if (!pixels_src) {
    fprintf(stderr, "ERROR > unpacking pixels from null source.\n");
    errno = EPERM;
    return 1;
  }
  if (!bytes_dest) {
    fprintf(stderr, "ERROR > unpacking pixels to null destination.\n");
    errno = EPERM;
    return 1;
  }
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++) {
      bytes_dest[r][c * 4] = pixels_src[r][c].r;
      bytes_dest[r][(c * 4) + 1] = pixels_src[r][c].g;
      bytes_dest[r][(c * 4) + 2] = pixels_src[r][c].b;
      bytes_dest[r][(c * 4) + 3] = pixels_src[r][c].a;
    }
  }
  return 0;
}

/**
 * copies the pixel data from one pngz to another.
 *
 * @param z_src source pixel buffer, passed by val
 * @param z_dest destination pixel buffer, passed by ptr
 * @return exit code
 */
int PNGZ_Copy(const PNGZ_Image z_src, PNGZ_Image* z_dest) {
  unsigned r, c;
  errno = 0;
  if (!z_src.pixels) {
    fprintf(stderr, "ERROR > copying from null source.\n");
    errno = EPERM;
    return 1;
  }
  if (!z_dest || !(z_dest->pixels)) {
    fprintf(stderr, "ERROR > copying to null destination.\n");
    errno = EPERM;
    return 1;
  }
  for (r = 0; r < z_src.height && r < z_dest->height; r++) {
    for (c = 0; c < z_src.width && c < z_dest->width; c++) {
      z_dest->pixels[r][c] = z_src.pixels[r][c];
    }
  }
  return 0;
}

/** load a pngz from a path directly passed into the call,
 * just a wrapper for standard pngz_load()
 *
 * @param z pngz* easy png ptr to load into
 * @param path char* path to read from
 * @return exit code
 */
int PNGZ_LoadFrom(PNGZ_Image* z, const char* path) {
  z->path = path;
  return PNGZ_Load(z);
}

/**
 * load a pngz object into memory
 *
 * @param z pngz* easy png ptr to load into
 * @return exit code
 */
int PNGZ_Load(PNGZ_Image* z) {
  FILE *fp;
  unsigned width, height;
  uint8_t bit_depth, color_type;
  uint8_t** byte_ptrs;
  png_structp png;
  png_infop info;
  errno = 0;
  /* defaults */
  z->path = z->path ? z->path : "default.png";
  fprintf(stderr, "pngz loading png at '%s'\n", z->path);
  /* open the file and read into info struct */
  if( ! (fp = fopen(z->path, "rb"))) {
    fprintf(stderr, "ERROR > opening file.\n");
    errno = EIO;
    return 1;
  }
  if ( ! (png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    fprintf(stderr, "ERROR > creating read struct.\n");
    errno = EIO;
    return 1;
  }
  if ( ! (info = png_create_info_struct(png))) {
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
  } else if (bit_depth < 8) {
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
  PNGZ_AllocBytes(&byte_ptrs, height, width * 4);
  PNGZ_AllocPixels(&(z->pixels), height, width);
  /* load into byte buffer, transfer to pixels */
  png_read_image(png, byte_ptrs);
  PNGZ_BytesToPixels(byte_ptrs, z->pixels, height, width);
  /* clean up */
  PNGZ_FreeBytes(byte_ptrs, height);
  png_destroy_read_struct(&png, &info, NULL);
  fclose(fp);
  /* if we got here, pack referenced struct and return */
  z->width  = width;
  z->height = height;
  return 0;
}

/**
 * write a png back out to file, wrapper for pngz_save_as()
 *
 * @param z pngz* easy png ptr to write to file
 * @return exit code
 */
int PNGZ_Save(const PNGZ_Image z) {
  return PNGZ_SaveAs(z, z.path);
}

/**
 * write a png back out to file with a new name
 *
 * @param path char* file path to save to
 * @param z pngz* easy png ptr to write to file
 * @return exit code
 */
int PNGZ_SaveAs(const PNGZ_Image z, const char* path) {
  FILE *fp;
  uint8_t** bytes;
  png_structp png;
  png_infop info;
  errno = 0;
  /* open the file and write info struct */
  if ( ! (fp = fopen(path, "wb"))) {
    fprintf(stderr, "ERROR > opening file.\n");
    errno = EIO;
    return 1;
  }
  if ( ! (png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    fprintf(stderr, "ERROR > creating write struct.\n");
    errno = EIO;
    return 1;
  }
  if ( ! (info = png_create_info_struct(png))) {
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
  PNGZ_AllocBytes(&bytes, z.height, z.width * 4);
  PNGZ_PixelsToBytes(z.pixels, bytes, z.height, z.width);
  png_write_image(png, bytes);
  png_write_end(png, NULL);
  /* clean up */
  PNGZ_FreeBytes(bytes, z.height);
  png_destroy_write_struct(&png, &info);
  fclose(fp);
  return 0;
}

/* printing functions *-------------------------------------------------------*/

/**
 * print a pngz ztruct contents.
 *
 * @param z pngz to print
 * @return exit code
 */
int PNGZ_PrintImage(const PNGZ_Image z) {
  printf("PNGZ [\n");
  printf("  rows x cols : %d x %d\n", z.height, z.width);
  if (z.pixels) {
    printf("  pixels : loaded\n");
  } else {
    printf("  pixels : empty\n");
  }
  printf("]\n");
  return 0;
}

/**
 * print out a single pixels rgba values in hex.
 *
 * @param p unsigned char* to the pixel to print
 * @return exit code
 *
 */
int PNGZ_PrintPixel(const PNGZ_Pixel p) {
  printf(
    "PIXEL [ #%02X%02X%02X%02X ]\n",
    p.r, p.g, p.g, p.a
  );
  return 0;
}

/* private util function to indent */
static void print_indent(const int indent) {
  int i;
  for (i = 0; i < indent; i++) {
    printf("  ");
  }
  return;
}

/**
 * print a pngz ztruct contents.
 *
 * @param z pngz to print
 * @param indent int for how much to indent this print
 * @return exit code
 */
int PNGZ_PrintImageIndent(const PNGZ_Image z, const int indent) {
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
  return 0;
}

/**
 * print out a single pixels rgba values.
 *
 * @param p unsigned char* to the pixel to print
 * @param indent int for how much to indent this print
 * @return exit code
 *
 */
int PNGZ_PrintPIxelIndent(const PNGZ_Pixel p, const int indent) {
  print_indent(indent);
  printf(
    "PIXEL [ #%02X%02X%02X%02X ]\n",
    p.r, p.g, p.g, p.a
  );
  return 0;
}
