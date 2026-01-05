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

### image (an "easy" png)
```c
typedef struct PNGZ_Image {
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
typedef struct PNGZ_Pixel {
  /** channel values 0x0-0xFF (0-255) */
  unsigned char r, g, b, a;
} pixel;
```

## example
```c
#include "pngz.h"

int main() {
  //load
  PNGZ_Image z;
  PNGZ_LoadFrom(&z, "./image.png");
  PNGZ_Print(z);
  // directly edit values
  z.pixels[0][0].r = 50;
  //save and free
  PNGZ_SaveAs(z, "./new_image.png");
  PNGZ_Free(&z);
}
```

## functions
all functions return an integer exit code (0 on success, 1 on failure) other than alloc functions which return a ptr that you can check for null, and set `errno` for specific failures that can be checked with `perror()`. errors also generate prints to `stderr`.


### allocate & free
allocating can be manually done to create pixel buffers from scratch, however `pngz_load()` handles its own allocation.
```c
PNGZ_Pixel** PNGZ_AllocPixels(unsigned rows, unsigned cols);
unsigned char** PNGZ_AllocBytes(unsigned rows, unsigned cols);
int PNGZ_FreePixels(PNGZ_Pixel** pixels, unsigned rows);
int PNGZ_FreeBytes(unsigned char** bytes, unsigned rows);
int PNGZ_Free(PNGZ_Image* z);
```

### load and save

```c
/* load and save */
int PNGZ_BytesToPixels(
  unsigned char** bytes_src, PNGZ_Pixel** pixels_dest,
  unsigned rows, unsigned cols
);
int PNGZ_PixelsToBytes(
  PNGZ_Pixel** pixels_src, unsigned char** bytes_dest,
  unsigned rows, unsigned cols
);
int PNGZ_Load(PNGZ_Image* z);
int PNGZ_LoadFrom(PNGZ_Image* z, char* path);
int PNGZ_Save(PNGZ_Image z);
int PNGZ_SaveAs(PNGZ_Image z, char* path);
int PNGZ_Copy(PNGZ_Image z_src, PNGZ_Image* z_dest);
```
### print
```c
int PNGZ_PrintImage(PNGZ_Image z);
int PNGZ_PrintPixel(PNGZ_Pixel p);
int PNGZ_PrintImageIndent(PNGZ_Image z, int indent);
int PNGZ_PrintPixelIndent(PNGZ_Pixel p, int indent);
```
