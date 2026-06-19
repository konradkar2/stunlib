#pragma once

#include "stun_attribute.hpp"

namespace stun {

class FingerPrintAttribute : public StunAttribute {
  uint16_t get_type() const override;
  uint16_t get_length() const override;
  size_t serialize(std::span<uint8_t> target) const override;
};

} // namespace stun
