#pragma once

#include "stun_message.hpp"
#include <netinet/in.h>

namespace stun {

enum class AdressFamily {
  IPv4 = 1,
  IPv6 = 2,
};

class MappedAddressAttribute : public StunAttribute {
  AdressFamily m_family;
  uint16_t m_port;
  union {
    uint32_t ipv4;
    uint32_t ipv6[4];
  } m_address;

public:
  AdressFamily getFamily() const;
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