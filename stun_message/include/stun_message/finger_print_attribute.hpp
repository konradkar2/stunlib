#pragma once

#include "stun_message/stun_attribute.hpp"

namespace stun {

class FingerPrintAttribute : public StunAttribute {
  static constexpr uint32_t k_fp_const = 0x5354554e;
  uint32_t m_finger_print;
public:
  FingerPrintAttribute() : StunAttribute(AttributeTypeId::FingerPrint) {}
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  uint32_t compute_finger_print(std::span<const uint8_t> message);
  void print_value() const override;
};

} // namespace stun
