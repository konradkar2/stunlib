#pragma once

#include <span>
#include <netinet/in.h>
#include <iomanip>

namespace stun {

enum class AttributeTypeId{
  MappingAddress = 0x0001,
  ErrorCode = 0x0009,
  XorMappingAddress = 0x0020,
  FingerPrint = 0x8028
};

#ifndef PRINT_ATTR_TYPE_ID
#define PRINT_ATTR_TYPE_ID
inline std::ostream& operator<<(std::ostream& os, AttributeTypeId typeId) {
  switch (typeId) {
    case AttributeTypeId::MappingAddress: return os << "MappingAddress";
    case AttributeTypeId::ErrorCode: return os << "ErrorCode";
    case AttributeTypeId::XorMappingAddress: return os << "XorMappingAddress";
    case AttributeTypeId::FingerPrint: return os << "FingerPrint";
  }
  return os << "unknown";
}
#endif

enum class AddressFamily {
  IPv4 = 1,
  IPv6 = 2,
};

class StunAttribute {
public:
  virtual AttributeTypeId get_type() const = 0;
  virtual uint16_t get_length() const = 0;
  virtual void deserialize(std::span<const uint8_t> source) = 0;
  virtual size_t serialize(std::span<uint8_t> target) const = 0;
  virtual void print_value() const = 0;
  virtual ~StunAttribute() = default;
};

} // namespace stun
