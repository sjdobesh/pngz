# pngz

an easy and portable png io interface for the standard `<png.h>` header that converts all images to 8bit RGBA pngs for basic pixel manipulation.
written as ANSI C (C89)

## full documentation
available [here](https://sjdobesh.github.io/pngz/index.html).


## dependencies
### linux
standard png library (`<png.h>`) and basic C dev tools (`gcc` and `make`), available through your choice of package manager.
```
sudo apt-get install libpng-dev build-essential
```
### windows
get a POSIX compliancy layer like [MSYS2](https://www.msys2.org/) which contains then `pacman` package manager.
In an admin powershell you can acquire dev tools through the package `base-devel`.
```
pacman -S base-devel
```
to find a suitable libpng library run `pacman -Ss libpng` to get a list of architectures available.
here is an example output.
```
pacman -Ss libpng
mingw32/mingw-w64-i686-libpng 1.6.51-1
    A collection of routines used to create PNG format graphics (mingw-w64)
mingw64/mingw-w64-x86_64-libpng 1.6.51-1
    A collection of routines used to create PNG format graphics (mingw-w64)
ucrt64/mingw-w64-ucrt-x86_64-libpng 1.6.51-1
    A collection of routines used to create PNG format graphics (mingw-w64)
clang64/mingw-w64-clang-x86_64-libpng 1.6.51-1
    A collection of routines used to create PNG format graphics (mingw-w64)
```

alternatively you could use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install).

## usage
the install script compiles a static and shared library into `./lib`. the shared library and header are copied to `/usr/local/lib` and `/usr/local/bin`.
you may also need to update `/etc/ld.so.conf` and run `ldconfig` for your system to recognize the library.

```
git clone https://sjdobesh.github/pngz.git
cd pngz
./install
```

include with `#include "pngz.h"` and link when compiling, including the standard `lpng` dependency.

```
gcc prog.c -lpngz -lpng
```

alternatively, you can compile the libraries with `make` and copy to a project location.
if both static and shared libraries are found, the linker gives preference to linking with the shared library unless the `-Bstatic` option is used.
```
make lib
cp ./lib/libpngz.so ~/yourproject/lib/ #shared
cp ./lib/libpngz.a ~/yourproject/lib/ #static
cp ./src/pngz.h ~/yourproject/include/
cd ~/yourproject
gcc -I./include/ -L./lib/ prog.c -lpngz -lpng #shared
gcc -I./include/ -L./lib/ -Bstatic prog.c -lpngz -lpng #static
```

finally, you can always simply copy `pngz.c` and `pngz.h` to your project and compile them with your other files.
```
cp ./src/pngz.* ~/yourproject/src/
cd ~/yourproject
gcc prog.c pngz.c pngz.h
```

## data structures

### pngz ("easy png")
```c
typedef struct pngz {
  /** default path to load from and save to */
  char* path;
  /** png height in pixels (Y domain)*/
  unsigned height;
  /** png width in pixels (X domain)*/
  unsigned width;
  /** [height][width] pixel buffer (Y, X)*/
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
allocating can be manually done to create pixel buffers from scratch, however `pngz_load()` handles its own allocation.
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
