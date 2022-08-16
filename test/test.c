#include "../src/spng.h"

int main() {
  spng s;
  spng_load("./png/Catgun.png", &s);
  printf("%u, %u\n", s.width, s.height);
  spng_print(s);
  spng_print_rgba(&s.pixels[0][0]);
  spng_free(&s);
  return 0;
}
