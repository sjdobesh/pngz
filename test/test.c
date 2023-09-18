#include "../src/pngz.h"

int main() {
  pngz z = { .path = "./png/BEEG.png" };
  pngz_load(&z);
  pngz_print(z);
  pngz_save_as(z, "./png/test.png");
  pngz_free(&z);

  return 0;
}
