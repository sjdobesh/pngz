#include "../src/spng.h"

int main() {
  spng s = { .path = "./png/Catgun.png" };
  spng_load(&s);
  printf("%u, %u\n", s.width, s.height);
  spng_print(s);
  spng_print_pixel(s.pixels[0][s.width - 1]);
  spng_save(s);
  spng_print(s);
  spng_free(&s);
  spng_print(s);
  return 0;
}
