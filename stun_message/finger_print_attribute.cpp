#include "finger_print_attribute.hpp"

namespace stun {

AttributeTypeId FingerPrintAttribute::get_type() const {
  return AttributeTypeId::FingerPrint;
}

uint16_t FingerPrintAttribute::get_length() const {
  return 4;
}

void FingerPrintAttribute::deserialize(std::span<const uint8_t> source) {
  (void)source;
  //TODO
}

size_t FingerPrintAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  //TODO
  return 0;
}

void FingerPrintAttribute::print_value() const {
  //TODO
}

} // namespace stun
