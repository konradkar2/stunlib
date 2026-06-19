#include "finger_print_attribute.hpp"

namespace stun {

uint16_t FingerPrintAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::FingerPrint);
}

uint16_t FingerPrintAttribute::get_length() const {
  return 4;
}

size_t FingerPrintAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  return 0;
}

} // namespace stun
