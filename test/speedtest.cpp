#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <cstdint>

#include "crc32.hpp"
#include "mledc.hpp"
#include "fletcher32.hpp"
#include "fletcher4.hpp"
#include "speedtest.h"

using namespace std;

namespace
{

template <size_t byteCount> //
struct speed_tester
{
  mt19937_64 rng = mt19937_64{0};
  static constexpr size_t testCount = (1 << 18) / byteCount;
  crc32 crc;
  using ary = array<uint8_t, byteCount>;
  uniform_int_distribution<uint16_t> dist //uint8_t にすると cl.exe でエラーになるので仕方なく。
      = decltype(dist){0, 255};
  using clock = chrono::steady_clock;
  static constexpr uint32_t mledc_init = 0xe68f45bf; // 単なる乱数
  static constexpr uint32_t mledc_mul = 0x45426bed;  // ２進数で、 1 の数が 16個の素数

  static constexpr uint16_t f32_sum1 = 30341; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f32_sum2 = 49807; // 2進数で 1 が 8 個ある 素数

  static constexpr uint16_t f4_s1 = 33739; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s2 = 21101; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s3 = 55621; // 2進数で 1 が 8 個ある 素数
  static constexpr uint16_t f4_s4 = 16607; // 2進数で 1 が 8 個ある 素数

  int64_t run_mledc(vector<ary> const &arrays)
  {
    std::uint32_t sum = 0;
    auto start = clock::now();
    for (auto const &a : arrays)
    {
      sum += mledc::calculate<mledc_init, mledc_mul>(a.data(), a.size());
    }
    auto end = clock::now();
    if (sum == 0)
    { // In order not to be killed by the optimizer
      cout << "I was surprised!\n";
    }
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }
  int64_t run_f32(vector<ary> const &arrays)
  {
    std::uint32_t sum = 0;
    auto start = clock::now();
    for (auto const &a : arrays)
    {
      sum += fletcher32::calculate<f32_sum1, f32_sum2>(a.data(), a.size());
    }
    auto end = clock::now();
    if (sum == 0)
    { // In order not to be killed by the optimizer
      cout << "I was surprised!\n";
    }
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }
  int64_t run_f4(vector<ary> const &arrays)
  {
    std::uint32_t sum = 0;
    auto start = clock::now();
    for (auto const &a : arrays)
    {
      sum += fletcher4::calculate<f4_s1, f4_s2, f4_s3, f4_s4>(a.data(), a.size());
    }
    auto end = clock::now();
    if (sum == 0)
    { // In order not to be killed by the optimizer
      cout << "I was surprised!\n";
    }
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }
  int64_t run_crc(vector<ary> const &arrays)
  {
    std::uint32_t sum = 0;
    auto start = clock::now();
    for (auto const &a : arrays)
    {
      sum += crc(a.data(), a.size());
    }
    auto end = clock::now();
    if (sum == 0)
    { // In order not to be killed by the optimizer
      cout << "I was surprised!\n";
    }
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
  }

  void run()
  {
    auto arrays = vector<ary>(testCount);
    for (auto &a : arrays)
    {
      for (auto &e : a)
      {
        e = static_cast<uint8_t>(dist(rng));
      }
    }
    // Preparation gymnastics
    run_mledc(arrays);
    run_crc(arrays);
    run_f32(arrays);
    run_f4(arrays);
    // Action!
    auto tick_n = run_mledc(arrays);
    auto tick_c = run_crc(arrays);
    auto tick_f = run_f32(arrays);
    auto tick_f4 = run_f4(arrays);
    // Act in a different order #1
    tick_c += run_crc(arrays);
    tick_f += run_f32(arrays);
    tick_f4 += run_f4(arrays);
    tick_n += run_mledc(arrays);
    // Act in a different order #2
    tick_f += run_f32(arrays);
    tick_f4 += run_f4(arrays);
    tick_n += run_mledc(arrays);
    tick_c += run_crc(arrays);
    // Act in a different order #3
    tick_f4 += run_f4(arrays);
    tick_n += run_mledc(arrays);
    tick_c += run_crc(arrays);
    tick_f += run_f32(arrays);

    std::cout
        << "['" << byteCount << "bytes × " << (testCount * 4) << " times', "
        << tick_n * 1e-3 << ", "
        << tick_c * 1e-3 << ", "
        << tick_f * 1e-3 << ", "
        << tick_f4 * 1e-3 << "],\n";
  }
};
} // namespace

void speedtest::run()
{
  std::cout << "['', 'mledc', 'crc32', 'fletcher32', 'fletcher4'],\n";
  speed_tester<16>().run();
  speed_tester<31>().run();
  speed_tester<64>().run();
  speed_tester<127>().run();
  speed_tester<256>().run();
}
