#include <cstdlib>

#include "speedtest.h"
#if defined __x86_64__
#include "noisetest.h"
#include "noisetest2.h"
#endif

int main( int argc, char const * argv[] ) {
  speedtest::run();
#if defined __x86_64__
  auto n = argc<2 ? 1 : std::atoi( argv[1] );
  noisetest::run(n);
  auto n2 = argc<3 ? 1 : std::atoi( argv[2] );
  noisetest2::run(n2);
#endif
}
