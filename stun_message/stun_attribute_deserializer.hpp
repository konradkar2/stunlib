#pragma once
#include "stun_attribute.hpp"
#include "xor_mapped_address_attribute.hpp"
#include "mapped_address_attribute.hpp"
#include "error_code_attribute.hpp"
#include "finger_print_attribute.hpp"
#include <iostream>

namespace stun {

class StunAttributeDeserializer {
private:
  static constexpr uint16_t k_type_length_size = 4;

  static std::vector<std::span<const uint8_t>> split_attributes(std::span<const uint8_t> serialized_data);

public:
  static std::vector<std::unique_ptr<StunAttribute>> deserialize(std::span<const uint8_t> serialized_data);

};

} // namespace stun
