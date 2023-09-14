# spng

a simplified interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation such as loading, saving, printing, and multithreaded filtering.

## dependencies
standard png library (`<png.h>`), available through apt. 
```
sudo apt-get install libpng-dev
```
on windows you can find the download [here](https://gnuwin32.sourceforge.net/packages/libpng.htm), or use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install).

## usage
compiles a static and shared library in `./lib`. the shared library and header are copied to `/usr/local/lib` and `/usr/local/bin`.

```
git clone https://sjdobesh.github/spng.git
./install
```
you may also need to run `ldconfig` or update `/etc/ld.so.conf` for your system to recognize the library.

include with `#include "spng.h"` and link when compiling

```
gcc foo.c -lspng -lpng
```

## data structures

### spng (simple png)
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
```

### pixel
coordinates are embedded in pixels to enable convolution filtering.
```c
typedef struct pixel {
  unsigned char r, g, b, a;
  union {
    unsigned height;
    unsigned row;
  };
  union {
    unsigned width;
    unsigned col;
  };
} pixel;
```

## example
```c
#include "spng.h"
#define THREADS 8

pixel filterfoo(pixel p) {
  /* do stuff... */
  return p;
}

int main() {
  //load
  spng s = {.path = "./image.png"};
  spng_load(&s);
  spng_print(s);
  // multithreaded filter map
  spng_filter_threaded(s, filterfoo, THREADS);
  // directly edit values
  s.pixels[0][0].r = 50;
  //save and free
  spng_save(s);
  spng_free(&s);
}
```

## functions

### allocate & free
allocating can be manually done to create pixel buffers from sctach, however `spng_load()` handles its own allocation.
```c
pixel** spng_alloc_pixels(unsigned rows, unsigned cols);
unsigned char** spng_alloc_bytes(unsigned rows, unsigned cols);
int spng_free_pixels(pixel** pixels, unsigned rows);
int spng_free_bytes(unsigned char** bytes, unsigned rows);
int spng_free(spng*_s);
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
int spng_load(spng*_s);
int spng_save(spng_s);
int spng_save_to(spng_s, char* path);
```

### utility functions
```c
int spng_filter(spng*_s, pixel(*filter)(pixel));
int spng_filter_threaded(spng* s, pixel(*filter)(pixel), unsigned thread_count);

```

### print
```c
void spng_print(spng_s);
void spng_print_pixel(pixel p);
```
