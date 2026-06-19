#pragma once

#include "stun_attribute.hpp"

namespace stun {

class FingerPrintAttribute : public StunAttribute {
public:
  AttributeTypeId get_type() const override;
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
