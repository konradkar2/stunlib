#pragma once
#include "stun_attributes.hpp"
#include <iostream>

namespace stun {

enum class AttributeTypeId{
  MappingAddress = 0x0001,
  ErrorCode = 0x0009,
  XorMappingAddress = 0x0020,
  FingerPrint = 0x8028
};

class StunAttributeDeserializer {
private:
  static std::vector<std::span<const uint8_t>> split_serialized_data(std::span<const uint8_t> serialized_data) {
    std::vector<std::span<const uint8_t>> result;
	size_t offset = 0;
    constexpr size_t type_length_size = 4;
	while (offset< serialized_data.size()) {
      uint16_t length = static_cast<uint16_t>(serialized_data[offset + 2]) | (static_cast<uint16_t>(serialized_data[offset + 3]));
      result.push_back(serialized_data.subspan(offset, offset + length + type_length_size));
	  offset += length + type_length_size;
	}
	return result;
  }

  static std::unique_ptr<StunAttribute> create_attribute(std::span<const uint8_t> serialized_attribute) {
    auto attribute = std::make_unique<StunAttribute>();

    attribute->type = (static_cast<uint16_t>(serialized_attribute[0]) << 8) |
                       static_cast<uint16_t>(serialized_attribute[1]);

    attribute->length = (static_cast<uint16_t>(serialized_attribute[2]) << 8) |
                         static_cast<uint16_t>(serialized_attribute[3]);

    attribute->value.assign(
        serialized_attribute.begin() + 4,
        serialized_attribute.end());

    return attribute;
  }

public:
  static std::vector<std::unique_ptr<StunAttribute>> deserialize(std::span<const uint8_t> serialized_data) {
    std::vector<std::unique_ptr<StunAttribute>> result;
    if (serialized_data.size() == 0) {
        return {};
    }
    auto splitted_data = split_serialized_data(serialized_data);
	for (const auto& attr : splitted_data) {
	  result.push_back(create_attribute(attr));
	}
	return result;
  }

};

}

