#include <cstdint>
#include <cstdio>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

#include "noisetest.h"
#include "crc32.hpp"
#include "fletcher32.hpp"
#include "fletcher4.hpp"
#include "mledc.hpp"

using namespace std;

namespace
{

struct codes_t
{
  uint32_t crc;
  uint32_t edc;
  uint32_t f32;
  uint32_t f4;
};

struct result_t
{
  int ok;
  int ng;
  result_t() : ok(0), ng(0) {}
  void accumulate(bool o) { ++(o ? ok : ng); }
};

struct results_t
{
  result_t crc;
  result_t edc;
  result_t f32;
  result_t f4;
};

struct tester
{
  mt19937_64 rng = mt19937_64{0};
  uniform_int_distribution<uint16_t>
      dist // uint8_t にすると cl.exe でエラーになるので仕方なく。
      = decltype(dist){0, 255};
  static constexpr uint32_t mledc_init = 0xe68f45bf; // 単なる乱数
  static constexpr uint32_t mledc_mul =
      0x45426bed; // ２進数で1の数が16個の素数
  static constexpr uint16_t f32_sum1 = 30341; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f32_sum2 = 49807; // 2進数で 1 が 8 個ある 素数

  static constexpr uint16_t f4_s1 = 33739; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s2 = 21101; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s3 = 55621; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s4 = 16607; // 2進数で 1 が 8 個ある 素数
  crc32 crc;

  results_t results;
  codes_t make_codes(vector<uint8_t> const &bin)
  {
    return {
        crc(bin.data(), bin.size()),
        mledc::calculate<mledc_init, mledc_mul>(bin.data(), bin.size()),
        fletcher32::calculate<f32_sum1, f32_sum2>(bin.data(), bin.size()),
        fletcher4::calculate<f4_s1, f4_s2, f4_s3, f4_s4>(bin.data(), bin.size())};
  }
  vector<uint8_t> make(size_t len)
  {
    vector<uint8_t> r(len);
    for (auto &e : r)
    {
      e = static_cast<uint8_t>(dist(rng));
    }
    return r;
  }
  bool count(codes_t const &a, codes_t const &b)
  {
    results.crc.accumulate(a.crc != b.crc);
    results.edc.accumulate(a.edc != b.edc);
    results.f32.accumulate(a.f32 != b.f32);
    results.f4.accumulate(a.f4 != b.f4);
    return a.crc != b.crc && a.edc != b.edc && a.f32 != b.f32 && a.f4 != b.f4;
  }
  void test_all_1(size_t len)
  {
    auto org = make(len);
    auto codes = make_codes(org);
    for (size_t p0 = 0; p0 < len * 8; ++p0)
    {
      auto m = org;
      m[p0 / 8] ^= (1 << (p0 % 8));
      auto mcodes = make_codes(m);
      if (!count(codes, mcodes))
      {
        debug_out(len, "1", codes, mcodes);
      }
    }
  }

  void debug_out(size_t len, std::string const &title, codes_t const &o, codes_t const &m)
  {
    printf("\n%s: len=%zu edc=(%08x, %08x), crc=(%08x, %08x), f32=(%08x, %08x), f4=(%08x, %08x)\n", //
           title.c_str(), len, o.edc, m.edc, o.crc, m.crc, o.f32, m.f32, o.f4, m.f4);
  }

  void test_all_2(size_t len)
  {
    auto const org = make(len);
    auto codes = make_codes(org);
    for (size_t p0 = 0; p0 < len * 8; ++p0)
    {
      for (size_t p1 = 0; p1 < p0; ++p1)
      {
        auto m = org;
        m[p0 / 8] ^= (1 << (p0 % 8));
        m[p1 / 8] ^= (1 << (p1 % 8));
        auto mcodes = make_codes(m);
        if (!count(codes, mcodes))
        {
          debug_out(len, "2", codes, mcodes);
        }
      }
    }
  }
  using key_t = unordered_set<size_t>;
  struct hasher
  {
    size_t operator()(key_t const &k) const
    {
      size_t r = 0;
      for (auto const e : k)
      {
        r = r * 5 + e; // よく意味のわからない計算をしてみる
      }
      return r;
    }
  };
  size_t calc_trial_count(size_t len, size_t noise)
  {
    size_t n = 1;
    size_t d = 1;
    constexpr size_t LIM = 100;
    if (2 <= noise && 3 <= len)
    {
      return LIM;
    }
    for (size_t i = 0; i < noise; ++i)
    {
      n *= len * 8 - i;
      d *= i + 1;
    }
    return std::min<size_t>(LIM, n / d);
  }

  void test_fixedbyte(size_t len, size_t noise)
  {
    auto const org = make(len);
    auto m = org; // ループ内でのメモリ確保を避けるために使い回す
    auto codes = make_codes(org);
    auto start_dist = uniform_int_distribution<size_t>{0, len - 1};
    auto val_dist = uniform_int_distribution<std::uint8_t>{0, 255};
    size_t trial_count = 100;
    for (size_t trial = 0; trial < trial_count; ++trial)
    {
      std::copy(org.cbegin(), org.cend(), m.begin());
      auto start = start_dist(rng);
      auto val = [&]() -> std::uint8_t {
        for (;;)
        {
          auto v = val_dist(rng);
          auto uniq = [&]() -> bool {
            for (size_t ix = 0; ix < noise; ++ix)
            {
              if (v == org[ix % org.size()])
              {
                return false;
              }
            }
            return true;
          }();
          if (uniq)
          {
            return v;
          }
        }
      }();
      for (size_t ix = 0; ix < noise; ++ix)
      {
        m[ix % m.size()] = val;
      }
      auto mcodes = make_codes(m);
      if (!count(codes, mcodes))
      {
        debug_out(len, "b" + std::to_string(noise), codes, mcodes);
      }
    }
  }

  void test_some(size_t len, size_t noise)
  {
    unordered_set<key_t, hasher> done;
    uniform_int_distribution<size_t> posdist =
        decltype(posdist){0, len * 8 - 1};
    auto const org = make(len);
    auto m = org; // ループ内でのメモリ確保を避けるために使い回す
    auto codes = make_codes(org);
    size_t trial_count = calc_trial_count(len, noise);
    while (done.size() < trial_count)
    {
      key_t key;
      while (key.size() < noise)
      {
        key.insert(posdist(rng));
      }
      if (done.count(key))
      {
        continue;
      }
      done.insert(key);
      std::copy(org.cbegin(), org.cend(), m.begin());
      for (auto pos : key)
      {
        m[pos / 8] ^= (1 << (pos % 8));
      }
      auto mcodes = make_codes(m);
      if (!count(codes, mcodes))
      {
        debug_out(len, std::to_string(noise), codes, mcodes);
      }
    }
  }
  void shorter_test(size_t len)
  {
    test_all_1(len);
    test_all_2(len);
    size_t lim = std::min<size_t>(32, len * 8 / 3);
    for (size_t noise = 3; noise < lim; ++noise)
    {
      test_some(len, noise);
    }
  }
  void longer_test(size_t len)
  {
    size_t lim = std::min<size_t>(32, len * 8 / 3);
    for (size_t noise = 1; noise < lim; ++noise)
    {
      test_some(len, noise);
    }
  }
  void fixedbyte_test(size_t len)
  {
    for (size_t noise = 1; noise < len / 4; ++noise)
    {
      test_fixedbyte(len, noise);
    }
  }
};
} // namespace

void noisetest::run()
{
  tester t;
  for (int i = 0; i < 100; ++i)
  {
    for (size_t len = 1; len < 5; ++len)
    {
      t.shorter_test(len);
      t.fixedbyte_test(len);
    }
    for (size_t len = 5; len < 256; ++len)
    {
      t.longer_test(len);
      t.fixedbyte_test(len);
    }
    cout << "." << flush;
  }
  cout << "\nmledc-ok,mledc-ng,crc-ok,crc-ng,fletcher32-ok,fletcher32-ng,fletcher4-ok,fletcher4-ng\n";
  cout //
      << t.results.edc.ok << "," << t.results.edc.ng << ","
      << t.results.crc.ok << "," << t.results.crc.ng << ","
      << t.results.f32.ok << "," << t.results.f32.ng << ","
      << t.results.f4.ok << "," << t.results.f4.ng << endl;
}
