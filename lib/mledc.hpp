#if !defined mledc_hpp_
#define mledc_hpp_

#include <cstdint>

namespace mledc
{
namespace detail
{

/** 32bit rotate right
 * @param[in] m rotate this value
 * @param[in] s rotate "s" count
 * @return rotated value
 * @note s should be greater than 0 and less than 32
 */
inline std::uint32_t ror(std::uint32_t m, int s)
{
  return ((m << (32 - s)) + (m >> s));
}

/** 32bit unsigned multiply-accumulate
 * @tparam mul 乗算値。
 * @param[in] m value to be multiplied
 * @param[in] a value to be added
 * @return m * mul + a
 */
template <std::uint32_t mul>
inline std::uint32_t mla(std::uint32_t m, std::uint32_t a)
{
  return m * mul + a;
}

/** エラー検出値を更新する
 * @tparam mul 乗算値。
 * @param[in, out] c エラー検出値
 * @param[in] val 新たな入力値
 */
template <std::uint32_t mul>
inline void update(std::uint32_t &c, std::uint16_t val)
{
  c += mla<mul>(ror(c, 31), val);
}

} // namespace detail

/** エラー検出値を計算する
 * @tparam init 初期値。0 にすると、0のみの入力が 0 になるので、2進数で0と1がいい感じに混ざっている値がいいんじゃないかと思う。
 * @tparam mul 乗数。2進数で0と1がいい感じに混ざっている 素数 がいいんじゃないかと思う。
 * @param[in] data 入力データへのポインタ。
 * @param[in] size 入力データのバイト数。
 * @return 計算されたエラー検出値。
 */
template <std::uint32_t init, std::uint32_t mul>
inline std::uint32_t calculate(std::uint8_t const *data, std::uint32_t size)
{
  std::uint32_t c = init;
  // NOTE: Endian dependent code
  auto p = reinterpret_cast<std::uint16_t const *>(data);
  for (std::uint32_t i = 0; i < size / 2; ++i)
  {
    detail::update<mul>(c, p[i]);
  }
  if (size & 1)
  {
    detail::update<mul>(c, data[size - 1]);
  }
  return c;
}

} // namespace mledc

#endif
