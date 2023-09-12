# spng
*a probably very bad simple png library.*

essentially a simplified interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation such as loading, saving, printing, and function mapping.

## dependencies
standard png library (`<png.h>`), available through apt. 
```
sudo apt-get install libpng-dev
```
on windows you can find the download [here](https://gnuwin32.sourceforge.net/packages/libpng.htm).

## usage
compiles a static and shared library in `./lib`. the shared library and header are copied to `/usr/local/lib` and `/usr/local/bin`.

```
git clone https://sjdobesh.github/spng.git
./install
```

include with `#include "spng.h"` and link when compiling

```
gcc foo.c -lspng -lpng
```

## the spng data structure
```c
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

typedef struct pixel {
  unsigned char r, g, b, a;
} pixel;
```

## example
```c
#include "spng.h"
//load
spng s = {.path = "./image.png"};
spng_load(&s);
spng_print(s);
// edit
s.pixels[0][0].r = 50;
//save and free
spng_save(s);
spng_free(&s);
```

## functions

### allocate & free
allocating can be manually done to create pixel buffers from sctach, however `spng_load()` handles its own allocation.
```c
pixel** spng_alloc_pixels(unsigned rows, unsigned cols);
unsigned char** spng_alloc_bytes(unsigned rows, unsigned cols);
int spng_free_pixels(pixel** pixels, unsigned rows);
int spng_free_bytes(unsigned char** bytes, unsigned rows);
int spng_free(spng* p);
```

### load and save

```c
int spng_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
);
int spng_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
);
int spng_load(spng* p);
int spng_save(spng p);
int spng_save_to(spng p, char* path);
```

### utility functions
```c
int spng_filter(spng* p, void(*filter)(pixel*));
```

### print
```c
void spng_print(spng p);
void spng_print_pixel(pixel p);
```
