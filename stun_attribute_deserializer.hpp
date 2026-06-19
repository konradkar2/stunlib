#pragma once
#include "stun_attributes.hpp"
#include <iostream>

namespace stun {

class StunAttributeDeserializer {
private:
  static constexpr uint16_t k_type_length_size = 4;

  static std::vector<std::span<const uint8_t>> split_attributes(std::span<const uint8_t> serialized_data) {
    std::vector<std::span<const uint8_t>> result;
	size_t offset = 0;
	while (offset < serialized_data.size()) {
      uint16_t length_raw;
	  std::memcpy(&length_raw, serialized_data.data() + 2, 2);
      uint16_t length;
      length = ntohs(length_raw);
	  result.push_back(serialized_data.subspan(offset, offset + length + k_type_length_size));
	  offset += length + k_type_length_size;
	}
	return result;
  }

public:
  static std::vector<std::unique_ptr<StunAttribute>> deserialize(std::span<const uint8_t> serialized_data) {
    std::vector<std::unique_ptr<StunAttribute>> result;
    auto attributes_vector = split_attributes(serialized_data);

	for (const auto& attr : attributes_vector) {
      uint16_t raw;
	  std::memcpy(&raw, attr.data(), 2);
      AttributeTypeId typeId;
      typeId = static_cast<AttributeTypeId>(ntohs(raw));
      
	  switch (typeId) {
		case AttributeTypeId::XorMappingAddress: {
          auto new_attr = std::make_unique<XorMappedAddressAttribute>();
          new_attr->deserialize(attr.subspan(k_type_length_size));
	      result.push_back(std::move(new_attr));
          break;
        }
		default: {
		  break;
        }
	  }
	}
	return result;
  }

};

}

