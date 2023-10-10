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

## further documentation 

[structures](https://sjdobesh.github.io/pngz/annotated.html)

[functions](https://sjdobesh.github.io/pngz/globals_func.html)
