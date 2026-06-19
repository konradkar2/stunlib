#include "stun_attributes.hpp"
#include <cstdint>
#include <cstring>

namespace stun {

MappedAddressAttribute MappedAddressAttribute::create_v4(uint16_t port,
                                                         uint32_t address) {
  MappedAddressAttribute ret{};

  ret.m_family = AddressFamily::IPv4;
  ret.m_port = port;
  ret.m_address.ipv4 = address;

  return ret;
}

MappedAddressAttribute MappedAddressAttribute::create_v6(uint16_t port,
                                                         uint32_t address[4]) {
  MappedAddressAttribute ret{};

 ret.m_family = AddressFamily::IPv6;
  ret.m_port = port;
  memcpy(&ret.m_address.ipv6, address, sizeof(ret.m_address.ipv6));

  return ret;
}

uint16_t MappedAddressAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::MappingAddress);
}

uint16_t MappedAddressAttribute::get_length() const {
  return 4 + (get_family() == AddressFamily::IPv4 ? 1 : 4);
}

void MappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  std::memcpy(&m_family, source.data() + 1, 1);
}

size_t MappedAddressAttribute::serialize(std::span<uint8_t> target) const {
  size_t offset = 0;

  uint8_t padding = 0;
  std::memcpy(target.data() + offset, &padding, sizeof(padding));
  offset += sizeof(padding);

  uint8_t family = static_cast<uint8_t>(m_family);
  std::memcpy(target.data() + offset, &family, sizeof(family));
  offset += sizeof(family);

  uint16_t port = htons(m_port);
  std::memcpy(target.data() + offset, &port, sizeof(port));
  offset += sizeof(port);

  if (m_family == AddressFamily::IPv4) {
    uint32_t address = htonl(m_address.ipv4);
    std::memcpy(target.data() + offset, &address, sizeof(address));
    offset += sizeof(address);
  } else {
    uint32_t address[4] = {};
    for (int i = 0; i < 4; ++i) {
      address[i] = htonl(m_address.ipv6[i]);
    }
    std::memcpy(target.data() + offset, &address, sizeof(address));
    offset += sizeof(address);
  }

  return offset;
}

AddressFamily MappedAddressAttribute::get_family() const{
    return m_family;
}

XorMappedAddressAttribute XorMappedAddressAttribute::create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie) {
  XorMappedAddressAttribute ret{};

  ret.m_mapped_attribute = MappedAddressAttribute::create_v4(port, address);
  ret.m_magic_cookie = magic_cookie;
                                            
  return ret;
}

XorMappedAddressAttribute XorMappedAddressAttribute::create_v6(uint16_t port, uint32_t address[4],
                                          uint32_t magic_cookie,
                                          uint32_t transaction_id[3]) {
  XorMappedAddressAttribute ret{};

  ret.m_mapped_attribute = MappedAddressAttribute::create_v6(port, address);
  ret.m_magic_cookie = magic_cookie;
  memcpy(ret.m_transaction_id, transaction_id, sizeof(m_transaction_id));                                   
                                            
  return ret;
}

uint16_t XorMappedAddressAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::XorMappingAddress);
}

uint16_t XorMappedAddressAttribute::get_length() const {
  return 4 + (m_mapped_attribute.get_family() == AddressFamily::IPv4 ? 1 : 4);
}

void XorMappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  std::memcpy(&m_family, source.data() + 1, 1);
  uint16_t xport_raw;
  std::memcpy(&xport_raw, source.data() + 2, 2);
  m_xport = ntohs(xport_raw);
  if (m_family == AddressFamily::IPv4) {
    uint32_t addres_raw;
    std::memcpy(&addres_raw, source.data() + 4, 4);
    m_xaddress.ipv4 = ntohs(addres_raw);
  }
  else {

  }
}

size_t XorMappedAddressAttribute::serialize(std::span<uint8_t> target) const {
  size_t size = m_mapped_attribute.serialize(target);

  constexpr size_t kPortOffset = 2;
  constexpr size_t kAddressOffset = 4;

  uint32_t magic_cookie = htonl(m_magic_cookie);
  const auto *magic_cookie_bytes =
      reinterpret_cast<const uint8_t *>(&magic_cookie);

  target[kPortOffset] ^= magic_cookie_bytes[0];
  target[kPortOffset + 1] ^= magic_cookie_bytes[1];

  if (m_mapped_attribute.get_family() == AddressFamily::IPv4) {
    for (size_t i = 0; i < sizeof(magic_cookie); ++i) {
      target[kAddressOffset + i] ^= magic_cookie_bytes[i];
    }
  } else {
    const auto *transaction_id_bytes =
        reinterpret_cast<const uint8_t *>(m_transaction_id);

    for (size_t i = 0; i < sizeof(magic_cookie); ++i) {
      target[kAddressOffset + i] ^= magic_cookie_bytes[i];
    }
    for (size_t i = 0; i < sizeof(m_transaction_id); ++i) {
      target[kAddressOffset + sizeof(magic_cookie) + i] ^=
          transaction_id_bytes[i];
    }
  }

  return size;
}

uint16_t ErrorCodeAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::ErrorCode);
}

uint16_t ErrorCodeAttribute::get_length() const {
  return 4;
}

size_t ErrorCodeAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  return 0;
}

uint16_t FingerPrintAttribute::get_type() const {
  return static_cast<uint16_t>(AttributeTypeId::FingerPrint);
}

uint16_t FingerPrintAttribute::get_length() const {
  return 4;
}

size_t FingerPrintAttribute::serialize(std::span<uint8_t> target) const {
  (void)target;
  return 0;
}

} // namespace stun

