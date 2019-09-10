#include "speedtest.h"
#if defined __x86_64__
#include "noisetest.h"
#endif

int main() {
  speedtest::run();
#if defined __x86_64__
  noisetest::run();
#endif
}
