#pragma once

#include "stun_attribute.hpp"

namespace stun {

class XorMappedAddressAttribute : public StunAttribute {
  AddressFamily m_family;
  uint16_t m_xport;
  union {
    uint32_t ipv4;
    uint32_t ipv6[4];
  } m_xaddress;

  uint32_t m_magic_cookie;
  uint32_t m_transaction_id[3];

public:
  XorMappedAddressAttribute() : StunAttribute(AttributeTypeId::XorMappingAddress) {}
  AddressFamily get_family() const;
  static XorMappedAddressAttribute create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie);
  static XorMappedAddressAttribute create_v6(uint16_t port, uint32_t address[4],
                                          uint32_t magic_cookie,
                                          uint32_t transaction_id[3]);
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
