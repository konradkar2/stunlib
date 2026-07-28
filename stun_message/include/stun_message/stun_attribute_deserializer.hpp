#pragma once
#include "stun_message/stun_attribute.hpp"
#include "stun_message/stun_message.hpp"
#include <memory>

namespace stun {

class StunAttributeDeserializer {
private:
  static constexpr uint16_t k_type_length_size = 4;

  static void split_attributes(std::span<const uint8_t> serialized_data,
                               std::vector<std::span<const uint8_t>> &result);

public:
  static std::vector<std::unique_ptr<StunAttribute>> deserialize(std::span<const uint8_t> serialized_data, const StunHeader& stun_header);

};

} // namespace stun
