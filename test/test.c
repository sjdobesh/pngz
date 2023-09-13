#include "../src/spng.h"

void filter_test(pixel* p) {
  p->r = 0xFF;
  p->g = 0;
  p->b = 0;
  p->a = 0xFF;
}

int main() {
  spng s = { .path = "./png/Catgun.png" };
  spng_load(&s);
  printf("%u, %u\n", s.width, s.height);
  spng_print(s);
  spng_print_pixel(s.pixels[0][s.width - 1]);
  apply_filter(&s, filter_test);
  spng_print_pixel(s.pixels[0][s.width - 1]);
  spng_save_as(s, "./png/filtertest.png");
  spng_print(s);
  spng_free(&s);
  spng_print(s);
  return 0;
}
