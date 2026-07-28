#pragma once

#include "stun_library/stun_attribute.hpp"
#include <string>
#include <utility>

namespace stun {

class ErrorCodeAttribute : public StunAttribute {
  uint16_t m_error_code;
  std::string m_reason_phrase;
public:
  ErrorCodeAttribute() : StunAttribute(AttributeTypeId::ErrorCode) {}
  ErrorCodeAttribute(uint16_t error_code, std::string phrase)
      : StunAttribute(AttributeTypeId::ErrorCode), m_error_code(error_code),
        m_reason_phrase(std::move(phrase)) {}
  static ErrorCodeAttribute create_error(uint16_t error_code, const std::string& phrase);
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
