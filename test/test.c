#include "../src/spng.h"
#include <sys/time.h>
#include <math.h>

/* nonsense test full of math for stress */
pixel filter_test(pixel p) {
  p.r *= sin(p.r) * sin(p.g * p.b)/1.5;
  p.g *= cos(p.g) / cos(p.r + p.b);
  p.b *= tan(p.b/5) * sin(p.r);
  p.a *= 0xFF;
  return p;
}

#define THREADS 12

int main() {
  /* timer set up */
  struct timeval t1, t2, t3;

  spng s = { .path = "./png/BEEG.png" };
  spng_load(&s);
  spng_print(s);

  gettimeofday(&t1, NULL);
  spng_filter(&s, filter_test);
  gettimeofday(&t2, NULL);
  spng_filter_threaded(&s, filter_test, THREADS);
  gettimeofday(&t3, NULL);

  double single = (t2.tv_sec - t1.tv_sec);
  single += (t2.tv_usec - t1.tv_usec) / 1000000.0;
  double multi = (t3.tv_sec - t2.tv_sec);
  multi += (t3.tv_usec - t2.tv_usec) / 1000000.0;
  printf("single threaded: %f seconds\n", single);
  printf("multi threaded: %f seconds\n", multi);

  spng_save_as(s, "./png/filtertest.png");
  spng_free(&s);

  return 0;
}
