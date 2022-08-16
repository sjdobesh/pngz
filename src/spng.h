/* spng.h
 * samantha jane
 * simple png
 *--------------------------------------------------------------------------80*/

#ifndef _SPNG_H_
#define _SPNG_H_

#include <png.h>

/**
 * simplified png structure.
 * always contains RGBA8 color.
 */
typedef struct spng {
  unsigned int width;
  unsigned int height;
  unsigned char** pixels;
} spng;

/* prototypes */
int spng_load(char* path, spng* p);
int spng_free(spng* p);
int spng_write(char* path, spng* p);
int spng_filter(spng* p, void(*filter)(unsigned char*));
void spng_print(spng p);
void spng_print_rgba(unsigned char*);

#endif
