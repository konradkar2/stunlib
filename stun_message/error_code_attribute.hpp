#pragma once

#include "stun_attribute.hpp"

namespace stun {

class ErrorCodeAttribute : public StunAttribute {
  uint16_t m_error_code;
  std::string m_reason_phrase;
public:
  ErrorCodeAttribute() : StunAttribute(AttributeTypeId::ErrorCode) {}
  static ErrorCodeAttribute create_error(uint16_t error_code, const std::string& phrase);
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
