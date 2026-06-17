#pragma once

#include <span>
#include <netinet/in.h>

namespace stun {

enum class AddressFamily {
  IPv4 = 1,
  IPv6 = 2,
};

class StunAttribute {
public:
  uint16_t type;
  uint16_t length;
  std::vector<uint8_t> value;
  
  virtual size_t serialize(std::span<uint8_t> target) const;
  virtual ~StunAttribute() = default;
};

class MappedAddressAttribute : public StunAttribute {
  AddressFamily m_family;
  uint16_t m_port;
  union {
    uint32_t ipv4;
    uint32_t ipv6[4];
  } m_address;

public:
  AddressFamily getFamily() const;
  static MappedAddressAttribute create_v4(uint16_t port, uint32_t address);
  static MappedAddressAttribute create_v6(uint16_t port, uint32_t address[4]);
  size_t serialize(std::span<uint8_t> target) const override;
};

class XorMappedAddressAttribute : public StunAttribute {
  MappedAddressAttribute m_mapped_attribute;
  uint32_t m_magic_cookie;
  uint32_t m_transaction_id[3];

public:
  static XorMappedAddressAttribute create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie);
  static XorMappedAddressAttribute create_v6(uint16_t port, uint32_t address[4],
                                          uint32_t magic_cookie,
                                          uint32_t transaction_id[3]);
  size_t serialize(std::span<uint8_t> target) const override;
};

class ErrorCodeAttribute : public StunAttribute {
  size_t serialize(std::span<uint8_t> target) const override;
};

class FingerPrintAttribute : public StunAttribute {
  size_t serialize(std::span<uint8_t> target) const override;
};

} // namespace stun

