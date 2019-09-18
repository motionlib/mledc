#include <cstdint>
#include <cstdio>
#include <iostream>
#include <random>
#include <utility>
#include <map>
#include <set>
#include <algorithm>
#include <vector>

#include "noisetest2.h"
#include "fletcher32.hpp"
#include "mledc.hpp"

using namespace std;

namespace
{
struct result
{
  int ok_;
  int ng_;
  result() : ok_(0), ng_(0) {}
};
std::ostream & operator<<( std::ostream & o, result const & r ){
  return o << "ok/ng:" << r.ok_ << "/" << r.ng_;
}


struct results
{
  result mledc_;
  result f32_;
};

std::ostream & operator<<( std::ostream & o, results const & r ){
  return o << "medc:" << r.mledc_ << "  f32:" << r.f32_;
}

using key_type = pair<int, int>;

using calc_type = uint32_t (*)(uint8_t const *data, uint32_t size);

struct totalizer
{
  mt19937_64 rng_ = mt19937_64{0};
  uniform_int_distribution<uint16_t> // uint8_t にすると cl.exe でエラーになるので仕方なく。
      val_dist_ = decltype(val_dist_){0, 255};
  uniform_int_distribution<int> bitpos_dist_;
  totalizer() = delete;
  int nc_;
  int len_;
  totalizer(int nc, int len) : nc_(nc), len_(len), bitpos_dist_(0,len*8-1) {}

  vector<uint8_t> create_random_data()
  {
    vector<uint8_t> r(len_);
    for( auto & e : r ){
      e = val_dist_(rng_);
    }
    return r;
  }
  void update_bitpos(vector<int> & v )
  {
    auto uniq = [](vector<int> const & v )->bool {
      for( size_t i=1 ; i<v.size() ; ++i ){
        if ( v[i-1]==v[i] ){
          return false;
        }
      }
      return true;
    };
    for(;;){
      for( auto & e : v ){
        e=bitpos_dist_(rng_);
      }
      sort( begin(v), end(v) );
      if (uniq(v)){
        return;
      }
    }
  }
  
  result impl(calc_type calc, int trial_count)
  {
    result r;
    vector<uint8_t> org = create_random_data();
    vector<uint8_t> mod = org;
    vector<int> bitpos(nc_);
    auto code = calc(org.data(), org.size());
    for (int tried = 0; tried < trial_count; ++tried)
    {
      if ( 0==(tried+1)%50 ){
        vector<uint8_t> org = create_random_data();
      }
      update_bitpos(bitpos);
      std::copy( cbegin(org),cend(org), begin(mod));
      for( auto pos : bitpos ){
        mod[pos/8] ^= (1<<(pos % 8) );
      }
      auto mcode = calc(mod.data(), mod.size());
      if ( mcode==code ) {
        ++r.ng_;
      } else {
        ++r.ok_;
      }
    }
    return r;
  }

  results operator()(int trial_count)
  {
    results r;
    r.mledc_ = impl(mledc::calculate<0xe68f45bf, 0x45426bed>, trial_count);
    r.f32_ = impl(fletcher32::calculate<30341, 49807>, trial_count);
    return r;
  }
};

} // namespace

void noisetest2::run(int trial_count)
{
  for (int nc = 1; nc < 17; ++nc)
  {
    for (int len = 16; len < 64; ++len)
    {
      auto r = totalizer(nc, len)(trial_count);
      std::cout << nc << ", " << len << ", " << r << std::endl;
      std::cerr << nc << ", " << len << ", " << r << std::endl;
    }
  }
}
