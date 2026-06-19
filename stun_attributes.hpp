#pragma once

#include <span>
#include <netinet/in.h>

namespace stun {

enum class AttributeTypeId{
  MappingAddress = 0x0001,
  ErrorCode = 0x0009,
  XorMappingAddress = 0x0020,
  FingerPrint = 0x8028
};

enum class AddressFamily {
  IPv4 = 1,
  IPv6 = 2,
};

class StunAttribute {
public:
  virtual uint16_t get_type() const = 0;
  virtual uint16_t get_length() const = 0;
  virtual void deserialize(std::span<const uint8_t> source) = 0;
  virtual size_t serialize(std::span<uint8_t> target) const = 0;
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
  AddressFamily get_family() const;
  static MappedAddressAttribute create_v4(uint16_t port, uint32_t address);
  static MappedAddressAttribute create_v6(uint16_t port, uint32_t address[4]);
  uint16_t get_type() const override;
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
};

class XorMappedAddressAttribute : public StunAttribute {
  AddressFamily m_family;
  uint16_t m_xport;
  union {
    uint32_t ipv4;
    uint32_t ipv6[4];
  } m_xaddress;

  MappedAddressAttribute m_mapped_attribute;
  uint32_t m_magic_cookie;
  uint32_t m_transaction_id[3];

public:
  static XorMappedAddressAttribute create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie);
  static XorMappedAddressAttribute create_v6(uint16_t port, uint32_t address[4],
                                          uint32_t magic_cookie,
                                          uint32_t transaction_id[3]);
  uint16_t get_type() const override;
  uint16_t get_length() const override;
  void deserialize(std::span<const uint8_t> source) override;
  size_t serialize(std::span<uint8_t> target) const override;
};

class ErrorCodeAttribute : public StunAttribute {
  uint16_t get_type() const override;
  uint16_t get_length() const override;
  size_t serialize(std::span<uint8_t> target) const override;
};

class FingerPrintAttribute : public StunAttribute {
  uint16_t get_type() const override;
  uint16_t get_length() const override;
  size_t serialize(std::span<uint8_t> target) const override;
};

} // namespace stun

