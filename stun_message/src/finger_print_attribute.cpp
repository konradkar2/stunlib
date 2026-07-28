#include "stun_message/finger_print_attribute.hpp"
#include <cstring>

namespace stun {
namespace {

constexpr uint16_t kFingerprintLength = sizeof(uint32_t);
constexpr uint32_t kInitialCrc32Value = 0xFFFFFFFF;
constexpr uint32_t kCrc32Polynomial = 0xEDB88320;
constexpr int kBitsPerByte = 8;

} // namespace

uint16_t FingerPrintAttribute::get_length() const {
  return kFingerprintLength;
}

void FingerPrintAttribute::deserialize(std::span<const uint8_t> source) {
  uint32_t value;
  std::memcpy(&value, source.data(), sizeof(value));
  m_finger_print = ntohl(value);
}

size_t FingerPrintAttribute::serialize(std::span<uint8_t> target) const {
  uint32_t value = htonl(m_finger_print);
  std::memcpy(target.data(), &value, sizeof(value));

  return kFingerprintLength;
}

uint32_t FingerPrintAttribute::compute_finger_print(std::span<const uint8_t> message) {
  m_finger_print = kInitialCrc32Value;
  for (uint8_t byte : message) {
    m_finger_print ^= byte;

    for (int i = 0; i < kBitsPerByte; i++) {
      if (m_finger_print & 1)
        m_finger_print = (m_finger_print >> 1) ^ kCrc32Polynomial;
      else
        m_finger_print >>= 1;
    }
  }
  m_finger_print ^= kInitialCrc32Value;
  m_finger_print ^= k_fp_const;
  return m_finger_print;
}

void FingerPrintAttribute::print_value() const {
  std::cout << "Fingerprint: 0x" << std::setw(4) << std::setfill('0') << m_finger_print << std::endl;
}

} // namespace stun
