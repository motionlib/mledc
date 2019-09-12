#if !defined fletcher32_hpp_
#define fletcher32_hpp_

#include <cstdint>

namespace fletcher32
{
namespace detail
{

inline
std::uint32_t modffff(std::uint32_t v)
{
  auto f = [](std::uint32_t x) -> std::uint32_t {
    return (x & 0xffff) + (x >> 16);
  };
  return f(f(v));
}

struct state
{
  std::uint32_t s1_;
  std::uint32_t s2_;
  explicit state(std::uint16_t s1, std::uint16_t s2)
      : s1_(s1), s2_(s2) {}
  void update(std::uint16_t val)
  {
    s1_ += val;
    s2_ += s1_;
  }
  std::uint32_t value() const
  {
    return (modffff(s2_) << 16) | modffff(s1_);
  }
};
} // namespace detail
/** エラー検出値を計算する
 * @tparam sum1 初期値。
 * @tparam sum2 初期値。
 * @param[in] data 入力データへのポインタ。
 * @param[in] size 入力データのバイト数。
 * @return 計算されたエラー検出値。
 */
template <std::uint16_t sum1, std::uint32_t sum2>
inline std::uint32_t calculate(std::uint8_t const *data, std::uint32_t size)
{
  detail::state s(sum1, sum2);
  // NOTE: Endian dependent code
  auto p = reinterpret_cast<std::uint16_t const *>(data);
  for (std::uint32_t i = 0; i < size / 2; ++i)
  {
    s.update(p[i]);
  }
  if (size & 1)
  {
    s.update(data[size - 1]);
  }
  return s.value();
}

} // namespace fletcher32

#endif
