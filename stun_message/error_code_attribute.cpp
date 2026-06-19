#include "error_code_attribute.hpp"

namespace stun {

uint16_t ErrorCodeAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::ErrorCode);
}

uint16_t ErrorCodeAttribute::get_length() const {
  return 4;
}

size_t ErrorCodeAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  return 0;
}

} // namespace stun
