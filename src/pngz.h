/* pngz.h
 * samantha jane
 * easy png
 *--------------------------------------------------------------------------80*/

#ifndef _PNGZ_H_
#define _PNGZ_H_

#include <png.h>
#include <stdint.h>
/**
 * pixel in RGBA8 format.
 */
typedef struct PNGZ_Pixel {
  /** channel values */
  uint8_t r, g, b, a;
} PNGZ_Pixel;

/**
 * easy png based image structure.
 * always contains RGBA8 color (0x0 - 0xFFFFFF).
 * pixel buffer is [height][width] pixels ([rows][cols])
 */
typedef struct PNGZ_Image {
  /** default path to load from and save to */
  const char* path;
  /** [height][width] pixel buffer (Y, X)*/
  PNGZ_Pixel** pixels;
  /** png height in pixels (Y domain)*/
  unsigned height;
  /** png width in pixels (X domain)*/
  unsigned width;
} PNGZ_Image;

/* prototypes */

/* allocate & free */
int PNGZ_AllocPixels(PNGZ_Pixel*** pixels_ptr, const unsigned rows, const unsigned cols);
int PNGZ_AllocBytes(uint8_t*** bytes_ptr, const unsigned rows, const unsigned cols);
int PNGZ_FreePixels(PNGZ_Pixel** pixels, const unsigned rows);
int PNGZ_FreeBytes(uint8_t** bytes, const unsigned rows);
int PNGZ_Free(PNGZ_Image* z);

/* load and save */
int PNGZ_BytesToPixels(
  uint8_t** bytes_src, PNGZ_Pixel** pixels_dest,
  const unsigned rows, const unsigned cols
);
int PNGZ_PixelsToBytes(
  PNGZ_Pixel** pixels_src, uint8_t** bytes_dest,
  const unsigned rows, const unsigned cols
);
int PNGZ_Load(PNGZ_Image* z);
int PNGZ_LoadFrom(PNGZ_Image* z, const char* path);
int PNGZ_Save(const PNGZ_Image z);
int PNGZ_SaveAs(const PNGZ_Image z, const char* path);
int PNGZ_Copy(const PNGZ_Image z_src, PNGZ_Image* z_dest);

/* print */
int PNGZ_PrintImage(const PNGZ_Image z);
int PNGZ_PrintPixel(const PNGZ_Pixel p);
int PNGZ_PrintImageIndent(const PNGZ_Image z, const int indent);
int PNGZ_PrintPixelIndent(const PNGZ_Pixel p, const int indent);

#endif
