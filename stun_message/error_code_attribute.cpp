#include "error_code_attribute.hpp"

namespace stun {

AttributeTypeId ErrorCodeAttribute::get_type() const {
  return AttributeTypeId::ErrorCode;
}

uint16_t ErrorCodeAttribute::get_length() const {
  return 4;
}

void ErrorCodeAttribute::deserialize(std::span<const uint8_t> source) {
  (void)source;
  //TODO
}

size_t ErrorCodeAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  //TODO
  return 0;
}

void ErrorCodeAttribute::print_value() const {
  //TODO
}

} // namespace stun
