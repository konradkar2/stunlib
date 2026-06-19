#include "xor_mapped_address_attribute.hpp"
#include <iostream>
#include <arpa/inet.h>

namespace stun {

AddressFamily XorMappedAddressAttribute::get_family() const{
    return m_family;
}

XorMappedAddressAttribute XorMappedAddressAttribute::create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie) {
  XorMappedAddressAttribute ret{};

  ret.m_magic_cookie = magic_cookie;
  ret.m_family = AddressFamily::IPv4;
  ret.m_xport = port;
  ret.m_xaddress.ipv4 = address ^ magic_cookie;
                                            
  return ret;
}

XorMappedAddressAttribute XorMappedAddressAttribute::create_v6(uint16_t port, uint32_t address[4],
                                          uint32_t magic_cookie,
                                          uint32_t transaction_id[3]) {
  XorMappedAddressAttribute ret{};

  ret.m_magic_cookie = magic_cookie;
  memcpy(ret.m_transaction_id, transaction_id, sizeof(m_transaction_id));      
  ret.m_family = AddressFamily::IPv6;
  ret.m_xport = port ^ static_cast<uint16_t>(magic_cookie >> 16);

  uint32_t xaddress[4];
  xaddress[0] = address[0] ^ magic_cookie;
  for (int i = 1; i < 4; i++) {
    xaddress[i] = address[i] ^ transaction_id[i - 1];
  }
  memcpy(&ret.m_xaddress.ipv6, xaddress, sizeof(ret.m_xaddress.ipv6));
                                  
                                            
  return ret;
}

AttributeTypeId XorMappedAddressAttribute::get_type() const {
  return AttributeTypeId::XorMappingAddress;
}

uint16_t XorMappedAddressAttribute::get_length() const {
  return 4 + (get_family() == AddressFamily::IPv4 ? 1 : 4);
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
    //TODO IPv6
  }
}

size_t XorMappedAddressAttribute::serialize(std::span<uint8_t> target) const {
  size_t offset = 0;

  uint8_t padding = 0;
  std::memcpy(target.data() + offset, &padding, sizeof(padding));
  offset += sizeof(padding);

  uint8_t family = static_cast<uint8_t>(m_family);
  std::memcpy(target.data() + offset, &family, sizeof(family));
  offset += sizeof(family);

  uint16_t port = htons(m_xport);
  std::memcpy(target.data() + offset, &port, sizeof(port));
  offset += sizeof(port);

  if (m_family == AddressFamily::IPv4) {
    uint32_t address = htonl(m_xaddress.ipv4);
    std::memcpy(target.data() + offset, &address, sizeof(address));
    offset += sizeof(address);
  } else {
    uint32_t address[4] = {};
    for (int i = 0; i < 4; ++i) {
      address[i] = htonl(m_xaddress.ipv6[i]);
    }
    std::memcpy(target.data() + offset, &address, sizeof(address));
    offset += sizeof(address);
  }

  return offset;
}

void XorMappedAddressAttribute::print_value() const {
  std::cout << "family: ";
  if (m_family == AddressFamily::IPv4) {
    std::cout << "IPv4\n";
  } else {
    std::cout << "IPv6\n";
  }

  std::cout << "xport: 0x" << m_xport << '\n';

  if (m_family == AddressFamily::IPv4) {
    std::cout << "xaddress: 0x" << std::setw(8) << std::setfill('0') << m_xaddress.ipv4 << std::endl;
  } else {
    //TODO
  }
}

} // namespace stun
