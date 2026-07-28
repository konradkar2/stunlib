#pragma once

#include "stun_library/ip_address.hpp"
#include "stun_library/stun_attribute.hpp"
#include <array>

namespace stun {

class MappedAddressAttribute : public StunAttribute {
  uint16_t m_port;
  IpAddress m_address;

public:
  MappedAddressAttribute()
      : StunAttribute(AttributeTypeId::MappingAddress), m_port(0),
        m_address(IpAddress::from_ipv4(0)) {}
  AddressFamily get_family() const;
  uint16_t get_port() const;
  IpAddress get_ip_address() const;
  static MappedAddressAttribute from_ip_address(uint16_t port,
                                                IpAddress address);
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
