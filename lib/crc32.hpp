// see https://ja.wikipedia.org/wiki/巡回冗長検査
#if !defined crc32_hpp_
#define crc32_hpp_

#include <cstdint>

class crc32 {
  std::uint32_t crc_table[256];

public:
  crc32() { make_crc_table(); }
  void make_crc_table() {
    for (std::uint32_t i = 0; i < 256; i++) {
      std::uint32_t c = i << 24;
      for (int j = 0; j < 8; j++) {
        c = (c << 1) ^ ((c & 0x80000000) ? 0x04C11DB7 : 0);
      }
      crc_table[i] = c;
    }
  }

  std::uint32_t operator()(uint8_t const *buf, int len) {
    std::uint32_t c = 0xffffffff;
    for (int i = 0; i < len; i++) {
      c = (c << 8) ^ crc_table[((c >> 24) ^ buf[i]) & 0xff];
    }
    return c;
  }
};

#endif
