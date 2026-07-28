#include "stun_message/finger_print_attribute.hpp"
#include <cstring>

namespace stun {

uint16_t FingerPrintAttribute::get_length() const {
  return 4;
}

void FingerPrintAttribute::deserialize(std::span<const uint8_t> source) {
  uint32_t value;
  std::memcpy(&value, source.data(), 4);
  m_finger_print = ntohl(value);
}

size_t FingerPrintAttribute::serialize(std::span<uint8_t> target) const {
  uint32_t value = htonl(m_finger_print);
  std::memcpy(target.data(), &value, 4);

  return 4;
}

uint32_t FingerPrintAttribute::compute_finger_print(std::span<const uint8_t> message) {
  m_finger_print = 0xFFFFFFFF;
  const uint32_t poly = 0xEDB88320;
  for (uint8_t byte : message) {
    m_finger_print ^= byte;

    for (int i = 0; i < 8; i++) {
      if (m_finger_print & 1)
        m_finger_print = (m_finger_print >> 1) ^ poly;
      else
        m_finger_print >>= 1;
    }
  }
  m_finger_print ^= 0xFFFFFFFF;
  m_finger_print ^= k_fp_const;
  return m_finger_print;
}

void FingerPrintAttribute::print_value() const {
  std::cout << "Fingerprint: 0x" << std::setw(4) << std::setfill('0') << m_finger_print << std::endl;
}

} // namespace stun
