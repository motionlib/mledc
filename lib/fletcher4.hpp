#if !defined fletcher4_hpp_
#define fletcher4_hpp_

#include <cstdint>
#include <algorithm>

namespace fletcher4
{
namespace detail
{

inline uint32_t mod255(uint32_t a)
{
  return a % 255;
  // http://homepage.divms.uiowa.edu/~jones/bcd/mod.shtml#exmod15
  a = (a >> 16) + (a & 0xFFFF); /* sum base 2**16 digits */
  a = (a >> 8) + (a & 0xFF);    /* sum base 2**8 digits */
  if (a < 255)
    return a;
  if (a < (2 * 255))
    return a - 255;
  return a - (2 * 255);
}

inline uint32_t weak_mod255(uint32_t a)
{
  return (a >> 16) + (a & 0xFFFF);
}

struct state
{
  std::uint32_t s_[4];
  explicit state(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d)
      : s_{a, b, c, d} {}
  void update(std::uint16_t val)
  {
    s_[0] += val;
    s_[1] += s_[0];
    s_[2] += s_[1];
    s_[3] += s_[2];
  }
  void weak_mod()
  {
    for (auto &v : s_)
    {
      v = weak_mod255(v);
    }
  }
  std::uint32_t value() const
  {
    return (mod255(s_[0]) << 0) + (mod255(s_[1]) << 8) + (mod255(s_[2]) << 16) + (mod255(s_[3]) << 24);
  }
};
} // namespace detail
/** エラー検出値を計算する
 * @tparam sum1 初期値。
 * @tparam sum2 初期値。
 * @tparam sum3 初期値。
 * @tparam sum4 初期値。
 * @param[in] data 入力データへのポインタ。
 * @param[in] size 入力データのバイト数。
 * @return 計算されたエラー検出値。
 */
template <std::uint16_t sum1, std::uint32_t sum2, std::uint32_t sum3, std::uint32_t sum4>
inline std::uint32_t calculate(std::uint8_t const *data, std::uint32_t size)
{
  detail::state s(sum1,sum2,sum3,sum4);
  // NOTE: Endian dependent code
  auto p = reinterpret_cast<std::uint16_t const *>(data);
  constexpr std::uint32_t overflow_limit = 69;
  for (std::uint32_t i = 0; i < size / 2; i += overflow_limit)
  {
    std::uint32_t last = std::min<std::uint32_t>(size / 2, i + overflow_limit);
    for (std::uint32_t e = i; e < last; ++e)
    {
      s.update(p[e]);
    }
    s.weak_mod();
  }
  if (size & 1)
  {
    s.update(data[size - 1]);
  }
  return s.value();
}

} // namespace fletcher4

#endif
