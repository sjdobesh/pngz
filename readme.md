# pngz

an easy png io interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation.

## dependencies
standard png library (`<png.h>`), available through apt. 
```
sudo apt-get install libpng-dev
```
on windows you can find the download [here](https://gnuwin32.sourceforge.net/packages/libpng.htm), or use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install).

## usage
the install script compiles a static and shared library into `./lib`. the shared library and header are copied to `/usr/local/lib` and `/usr/local/bin`.
you may also need to run `ldconfig` or update `/etc/ld.so.conf` for your system to recognize the library.

```
git clone https://sjdobesh.github/pngz.git
cd pngz
./install
```

alternatively, you can compile the libraries with make and copy to a project location.
```
make lib
cp ./lib/libpngz.so ~/yourproject/
cp ./lib/libpngz.a ~/yourproject/
```

finally, you can always simply copy `pngz.c` and `pngz.h` to your project.
```
cp ./src/pngz.* ~/yourproject/
```

include with `#include "pngz.h"` and link when compiling, including the png header dependency.

```
gcc prog.c -lpngz -lpng
```

## full documentation
available [here](https://sjdobesh.github.io/pngz/index.html).

## data structures

### pngz (simple png)
```c
typedef struct pngz {
  /* path to load and save to */
  char* path;
  /* dimensions */
  union {
    unsigned height;
    unsigned rows;
  };
  union {
    unsigned width;
    unsigned cols;
  };
  /* pixel buffer */
  pixel** pixels;
} pngz;
```

### pixel
```c
typedef struct pixel {
  /* channel values 0x0-0xFF (0-255) */
  unsigned char r, g, b, a;
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
int pngz_load_from(pngz* z, char* path);
int pngz_save(pngz z);
int pngz_save_to(pngz z, char* path);
```
### print
```c
void pngz_print(pngz z);
void pngz_print_pixel(pixel p);
```
