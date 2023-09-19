# pngz

an easy png io interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation.

## full documentation
available [here](https://sjdobesh.github.io/pngz/index.html).


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

include with `#include "pngz.h"` and link when compiling, including the png header dependency.

```
gcc prog.c -lpngz -lpng
```

alternatively, you can compile the libraries with `make` and copy to a project location.
if both static and shared libraries are found, the linker gives preference to linking with the shared library unless the `-static` option is used.
```
make lib
cp ./lib/libpngz.so ~/yourproject/lib/ #shared
cp ./lib/libpngz.a ~/yourproject/lib/ #static
cp ./src/pngz.h ~/yourproject/include/
cd ~/yourproject
gcc -I./include/ -L./lib/ prog.c -lpngz -lpng
```

finally, you can always simply copy `pngz.c` and `pngz.h` to your project and compile them into your project.
```
cp ./src/pngz.* ~/yourproject/src/
cd ~/yourproject
gcc prog.c pngz.c pngz.h
```

## data structures

### pngz (simple png)
```c
typedef struct pngz {
  /** default path to load from and save to */
  char* path;
  /** union alias for either height or rows */
  union { 
    unsigned height;
    unsigned rows;
  };
  /** union alias for either width or cols */
  union {
    unsigned width;
    unsigned cols;
  };
  /** [height][width] pixel buffer */
  pixel** pixels;
} pngz;
```

### pixel
```c
typedef struct pixel {
  /** channel values 0x0-0xFF (0-255) */
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
all functions that return type `int` are returning an exit code (0 on success, 1 on failure), and set `errno` for specific failures that can be checked with `perror()`. errors also generate prints to `stderr`.

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
