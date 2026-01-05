#include "../src/pngz.h"

int main() {
  PNGZ_Image z1, z2;
  PNGZ_LoadFrom(&z1, "./png/Catgun.png");
  PNGZ_LoadFrom(&z2, "./png/Catgun.png");
  PNGZ_PrintImage(z1);
  PNGZ_Copy(z1, &z2);
  PNGZ_SaveAs(z1, "./png/TEST.png");
  PNGZ_Free(&z1);

  return 0;
}
