#include "../src/pngz.h"

int main() {
  pngz z;
  pngz_load_from(&z, "./png/Catgun.png");
  pngz_print(z);
  pngz_save_as(z, "./png/TEST.png");
  pngz_free(&z);

  return 0;
}
