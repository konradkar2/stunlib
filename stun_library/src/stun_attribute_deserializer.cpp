#include "stun_library/stun_attribute_deserializer.hpp"
#include "stun_library/error_code_attribute.hpp"
#include "stun_library/finger_print_attribute.hpp"
#include "stun_library/mapped_address_attribute.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"
#include <memory>

namespace stun {
namespace {

constexpr size_t kAttributeTypeOffset = 0;
constexpr size_t kAttributeLengthOffset = 2;
constexpr size_t kAttributePaddingAlignment = 4;

size_t padded_attribute_length(size_t length) {
  return (length + (kAttributePaddingAlignment - 1)) &
         ~(kAttributePaddingAlignment - 1);
}

} // namespace

void StunAttributeDeserializer::split_attributes(std::span<const uint8_t> serialized_data,
                                                 std::vector<std::span<const uint8_t>> &result) {
  size_t offset = 0;
  while (offset < serialized_data.size()) {
    uint16_t length_raw;
    std::memcpy(&length_raw,
                serialized_data.data() + offset + kAttributeLengthOffset,
                sizeof(length_raw));
    uint16_t length = ntohs(length_raw);
    const size_t padded_length = padded_attribute_length(length);
    result.push_back(serialized_data.subspan(offset, k_type_length_size + length));
    offset += k_type_length_size + padded_length;
  }
}

std::vector<std::unique_ptr<StunAttribute>> StunAttributeDeserializer::deserialize(std::span<const uint8_t> serialized_data, 
                                                                                   const StunHeader& stun_header ) {
  std::vector<std::unique_ptr<StunAttribute>> result;
  std::vector<std::span<const uint8_t>> attributes_vector;
  split_attributes(serialized_data, attributes_vector);

  for (const auto& attr : attributes_vector) {
    uint16_t raw;
    std::memcpy(&raw, attr.data() + kAttributeTypeOffset, sizeof(raw));
    AttributeTypeId typeId;
    typeId = static_cast<AttributeTypeId>(ntohs(raw));
    
    switch (typeId) {
      case AttributeTypeId::MappingAddress: {
        auto new_attr = std::make_unique<MappedAddressAttribute>();
        new_attr->deserialize(attr.subspan(k_type_length_size));
        result.push_back(std::move(new_attr));
        break;
      }
      case AttributeTypeId::ErrorCode: {
        auto new_attr = std::make_unique<ErrorCodeAttribute>();
        new_attr->deserialize(attr.subspan(k_type_length_size));
        result.push_back(std::move(new_attr));
        break;
      }
      case AttributeTypeId::XorMappingAddress: {
        auto new_attr = std::make_unique<XorMappedAddressAttribute>();
        new_attr->deserialize(attr.subspan(k_type_length_size));
        new_attr->set_magic_cookie(stun_header.magic_cookie);
        new_attr->set_transaction_id(stun_header.transaction_id);
        result.push_back(std::move(new_attr));
        break;
      }
      case AttributeTypeId::FingerPrint: {
        auto new_attr = std::make_unique<FingerPrintAttribute>();
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

} // namespace stun
