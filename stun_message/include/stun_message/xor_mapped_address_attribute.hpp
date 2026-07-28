#pragma once

#include "stun_message/ip_address.hpp"
#include "stun_message/stun_attribute.hpp"
#include <array>

namespace stun {

class XorMappedAddressAttribute : public StunAttribute {
  uint16_t m_xport;
  IpAddress m_xaddress;

  uint32_t m_magic_cookie;
  uint32_t m_transaction_id[3];

public:
  XorMappedAddressAttribute()
      : StunAttribute(AttributeTypeId::XorMappingAddress), m_xport(0),
        m_xaddress(IpAddress::from_ipv4(0)), m_magic_cookie(0),
        m_transaction_id{0, 0, 0} {}
  AddressFamily get_family() const;
  uint16_t get_port() const;
  IpAddress get_ip_address() const;
  static XorMappedAddressAttribute from_ip_address(uint16_t port,
                                                   IpAddress address,
                                                   uint32_t magic_cookie);
  static XorMappedAddressAttribute from_ip_address(
      uint16_t port, IpAddress address, uint32_t magic_cookie,
      const uint32_t transaction_id[3]);
  void set_magic_cookie(const uint32_t magic_cookie);
  void set_transaction_id(const uint32_t transaction_id[3]);                                        
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
  void print_value() const override;
};

} // namespace stun
