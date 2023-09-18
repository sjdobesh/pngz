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

```
git clone https://sjdobesh.github/pngz.git
./install
```
you may also need to run `ldconfig` or update `/etc/ld.so.conf` for your system to recognize the library.

include with `#include "pngz.h"` and link when compiling, including the png header dependency.

```
gcc prog.c -lpngz -lpng
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

[structures](https://sjdobesh.github.io/pngz/html/annotated.html)

[functions](https://sjdobesh.github.io/pngz/html/globals_func.html)
