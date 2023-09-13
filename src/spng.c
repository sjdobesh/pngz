/* spng.c
 * samantha jane
 * simple png library to handle loading, saving, filtering, and printing.
 *
 * normal use case:
 *
 * // load in an image and resave it elsewhere
 * spng s = {.path = "./img.png"};
 * spng_load(&s");
 * spng_save(&s");
 * spng_save_as(s, "./TEST.png");
 * spng_free(&s);
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
#include "spng.h"

/* multithreaded test ground *------------------------------------------------*/
#define THREADS 4
pthread_barrier_t thread_barrier;

void* threadfn(void* params) {
  struct thread_parameters* tp = (thread_parameters*) params;
  printf("[THREAD START] > %d\n", tp->thread_id);
  printf("[THREAD PARAMS]\n> row_start: %ld\n> row_end: %ld\n\n", tp->row_start, tp->row_stop);
  /* apply the filter here */
  for (unsigned i = tp->row_start; i < tp->row_stop; i++)
    for (unsigned j = 0; j < tp->image->width; j++)
      tp->filter_function(&(tp->image->pixels[i][j]));
  /* halt threads */
  pthread_barrier_wait(&thread_barrier);
  return NULL;
}

void apply_filter(spng* s, void (*filter) (pixel*)) {

  /* thread containers and barrier */
  pthread_t threads[THREADS];
  thread_parameters tps[THREADS];

  /* set barrier */
  pthread_barrier_init(&thread_barrier, NULL, THREADS);

  /* calc how much to do */
  unsigned divisions = s->height / THREADS;

  /* launch threads */
  for (int i = 0; i < THREADS; i++) {
    /* initialize thread params here */
    tps[i] = (thread_parameters) {
      .image = s, 
      .filter_function = filter,
      .row_start = i * divisions,
      .row_stop = (i + 1) * divisions > s->height ? s->height : (i+1) * divisions
    };
    if (pthread_create(&threads[i], NULL, threadfn, (void*)&tps[i])) {
      fprintf(stderr, "[ERROR] unable to create thread %d\n", i);
    }
  }

  /* join threads and remove barrier */
  for (int i = 0; i < THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  pthread_barrier_destroy(&thread_barrier);
}

/* alloc & free *-------------------------------------------------------------*/

/**
 * allocate a pixel buffer.
 * every pixel will be represented as a struct
 *
 * @param rows number of rows to allocate
 * @param cols number pixels per col to allocate
 * @return exit code, setting errno on failure
 */
pixel** spng_alloc_pixels(unsigned rows, unsigned cols) {
  unsigned i;
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
  for (i = 0; i < rows; i++) {
    pixels[i] = malloc(sizeof(pixel) * cols);
    if (!pixels[i]) {
      fprintf(stderr, "ERROR > allocing pixel buffer cols.\n");
      errno = ENOMEM;
      return NULL;
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
unsigned char** spng_alloc_bytes(unsigned rows, unsigned cols) {
  unsigned i;
  unsigned char** bytes;
  errno = 0;
  /* allocate rows to hold addresses */
  bytes = malloc(sizeof(void *) * rows);
  if (!bytes) {
    fprintf(stderr, "ERROR > allocing raw buffer rows.\n");
    errno = ENOMEM;
    return NULL;
  }
  for (i = 0; i < rows; i++) {
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
int spng_free_pixels(pixel** pixels, unsigned rows) {
  unsigned i;
  int exit_code;
  errno = 0;
  exit_code = 0;
  for (i = 0; i < rows; i++) {
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
int spng_free_bytes(unsigned char** bytes, unsigned rows) {
  unsigned i;
  int exit_code;
  errno = 0;
  exit_code = 0;
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
 * free a spng structs pixels.
 *
 * @param img png* loaded into memory
 * @return exit code, error if you try to free a NULL ptr
 */
int spng_free(spng* s) {
  return spng_free_pixels(s->pixels, s->height);
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
int spng_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
) {
  unsigned r, c;
  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++) {
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
int spng_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
) {
  unsigned r, c;
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
 * load a spng object into memory
 *
 *
 * @param p spng* simple png ptr to load into
 * @return exit code
 */
int spng_load(spng* s) {
  FILE *fp;
  png_structp png;
  png_infop info;
  unsigned width, height;
  unsigned char bit_depth, color_type;
  unsigned char** byte_ptrs;
  errno = 0;

  /* defaults */
  s->path = s->path ? s->path : "default.png";

  fprintf(stderr, "spng loading png at '%s'\n", s->path);


  /* open the file and read into info struct */
  if(!(fp = fopen(s->path, "rb"))) {
    fprintf(stderr, "ERROR > opening file.\n");
    errno = EIO;
    return 1;
  }
  if (!(png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
    fprintf(stderr, "ERROR > creating read struct.\n");
    errno = EIO;
    return 1;
  }
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

  /* malloc pixel buffers */
  byte_ptrs = spng_alloc_bytes(height, width * 4);
  s->pixels = spng_alloc_pixels(height, width);
  /* load into byte buffer, transfer to pixels */
  png_read_image(png, byte_ptrs);
  spng_pack_pixels(byte_ptrs, s->pixels, height, width);
  /* clean up */
  spng_free_bytes(byte_ptrs, height);
  png_destroy_read_struct(&png, &info, NULL);
  fclose(fp);

  /* if we got here, pack referenced struct and return */
  s->width  = width;
  s->height = height;
  return 0;
}

/**
 * write a png back out to file
 *
 * @param p spng* simple png ptr to write to file
 * @return exit code
 */
int spng_save(spng s) {
  return spng_save_as(s, s.path);
}

/**
 * write a png back out to file with a new name
 *
 * @param path char* file path to save to
 * @param p spng* simple png ptr to write to file
 * @return exit code
 */
int spng_save_as(spng s, char* path) {

  FILE *fp;
  png_structp png;
  png_infop info;
  unsigned char** bytes;

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
    s.width, s.height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);
  bytes = spng_alloc_bytes(s.height, s.width * 4);
  spng_unpack_pixels(s.pixels, bytes, s.height, s.width);
  png_write_image(png, bytes);
  png_write_end(png, NULL);

  /* clean up */
  spng_free_bytes(bytes, s.height);
  png_destroy_write_struct(&png, &info);
  fclose(fp);
  return 0;
}

/**
 * simple png filter.
 * maps a function onto every pixel.
 * errors if spng isn't loaded
 *
 * @param p spng* simple png to filter
 * @param filter void(*)(unsigned char*), a filter function that take pixel ptr
 * @return exit code
 */
int spng_filter(spng* s, void(*filter)(pixel*)) {
  unsigned row, col;
  if (!s->pixels) {
    fprintf(stderr, "ERROR > filtering failed due to no loaded image.\n");
    errno = EPERM;
    return 1;
  }
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
  printf("  row x col : %d x %d\n", s.height, s.width);
  if (s.pixels) {
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
void spng_print_pixel(pixel p) {
  printf(
    "PIXEL [ #%02X%02X%02X%02X ]\n",
    p.r, p.g, p.g, p.a
  );
  return;
}

/* private util function to indent */
void print_indent(int indent) {
  int i;
  for (i = 0; i < indent; i++) {
    printf("  ");
  }
  return;
}

/**
 * print a spng struct contents.
 *
 * @param p spng to print
 * @return void
 */
void spng_print_indent(spng s, int indent) {
  print_indent(indent);
  printf("SPNG [\n");
  print_indent(indent);
  printf("  row x col : %d x %d\n", s.height, s.width);
  print_indent(indent);
  if (s.pixels) {
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
void spng_print_pixel_indent(pixel p, int indent) {
  print_indent(indent);
  printf(
    "PIXEL [ #%02X%02X%02X%02X ]\n",
    p.r, p.g, p.g, p.a
  );
  return;
}
