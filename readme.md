# pngz

an easy png io interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation.

## dependencies
standard png library (`<png.h>`), available through apt. 
```
sudo apt-get install libpng-dev
```
on windows you can find the download [here](https://gnuwin32.sourceforge.net/packages/libpng.htm), or use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install).

## usage
compiles a static and shared library in `./lib`. the shared library and header are copied to `/usr/local/lib` and `/usr/local/bin`.

```
git clone https://sjdobesh.github/pngz.git
./install
```
you may also need to run `ldconfig` or update `/etc/ld.so.conf` for your system to recognize the library.

include with `#include "pngz.h"` and link when compiling

```
gcc foo.c -lpngz -lpng
```

## data structures

### pngz (simple png)
```c
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
#include "pngz.h"

int main() {
  //load
  pngz_load_from(&z, "./image.png");
  pngz_print(z);
  // directly edit values
  z.pixels[0][0].r = 50;
  //save and free
  pngz_save_as(z, "./new_image.png");
  pngz_free(&z);
}
```

## functions

### allocate & free
allocating can be manually done to create pixel buffers from sctach, however `pngz_load()` handles its own allocation.
```c
pixel** pngz_alloc_pixels(unsigned rows, unsigned cols);
unsigned char** pngz_alloc_bytes(unsigned rows, unsigned cols);
int pngz_free_pixels(pixel** pixels, unsigned rows);
int pngz_free_bytes(unsigned char** bytes, unsigned rows);
int pngz_free(pngz* z);
```

### load and save

```c
int pngz_pack_pixels(
  unsigned char** bytes_src, pixel** pixels_dest,
  unsigned rows, unsigned cols
);
int pngz_unpack_pixels(
  pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
);
int pngz_load(pngz* z);
int pngz_save(pngz z);
int pngz_save_to(pngz z, char* path);
```
### print
```c
void pngz_print(pngz z);
void pngz_print_pixel(pixel p);
```
