#pragma once

#include "stun_attribute.hpp"

namespace stun {

class MappedAddressAttribute : public StunAttribute {
  AddressFamily m_family;
  uint16_t m_port;
  union {
    uint32_t ipv4;
    uint32_t ipv6[4];
  } m_address;

public:
  MappedAddressAttribute() : StunAttribute(AttributeTypeId::MappingAddress) {}
  AddressFamily get_family() const;
  uint32_t get_ipv4_address() const;
  std::array<uint32_t,4> get_ipv6_address() const;
  static MappedAddressAttribute create_v4(uint16_t port, uint32_t address);
  static MappedAddressAttribute create_v6(uint16_t port, uint32_t address[4]);
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
