#include "stun_message/mapped_address_attribute.hpp"
#include <iostream>
#include <cstring>
#include <iterator>

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
                                                         const uint32_t address[4]) {
  MappedAddressAttribute ret{};

  ret.m_family = AddressFamily::IPv6;
  ret.m_port = port;
  memcpy(&ret.m_address.ipv6, address, sizeof(ret.m_address.ipv6));

  return ret;
}

uint16_t MappedAddressAttribute::get_length() const {
  return static_cast<uint16_t>(4 + (get_family() == AddressFamily::IPv4 ? sizeof(m_address.ipv4) : sizeof(m_address.ipv6)));
}

void MappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  uint8_t family;
  std::memcpy(&family, source.data() + 1, 1);
  m_family = static_cast<AddressFamily>(family);
  uint16_t xport_raw;
  std::memcpy(&xport_raw, source.data() + 2, 2);
  m_port = ntohs(xport_raw);
  if (m_family == AddressFamily::IPv4) {
    uint32_t addres_raw;
    std::memcpy(&addres_raw, source.data() + 4, 4);
    m_address.ipv4 = ntohl(addres_raw);
  }
  else if (m_family == AddressFamily::IPv6) {
    uint32_t address_raw[4] = {};
    std::memcpy(&address_raw, source.data() + 4, 16);
    for (size_t i = 0; i < std::size(m_address.ipv6); ++i) {
      m_address.ipv6[i] = ntohl(address_raw[i]);
    }
  }
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
    for (size_t i = 0; i < std::size(m_address.ipv6); ++i) {
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

uint32_t MappedAddressAttribute::get_ipv4_address() const {
  assert(m_family == AddressFamily::IPv4);
  return m_address.ipv4;
}


std::array<uint32_t,4> MappedAddressAttribute::get_ipv6_address() const {
  assert(m_family == AddressFamily::IPv6);
  return {m_address.ipv6[0], m_address.ipv6[1],
            m_address.ipv6[2], m_address.ipv6[3]};
}

void MappedAddressAttribute::print_value() const {
  std::cout << "family: ";
  if (m_family == AddressFamily::IPv4) {
    std::cout << "IPv4\n";
  } 
  else if (m_family == AddressFamily::IPv6) {
    std::cout << "IPv6\n";
  }

  std::cout << "xport: 0x" << m_port << '\n';

  if (m_family == AddressFamily::IPv4) {
    std::cout << "address: 0x" << std::setw(8) << std::setfill('0') << m_address.ipv4 << " : ";
    std::cout << " (decoded): "
            << ((m_address.ipv4 >> 24) & 0xFF) << '.'
            << ((m_address.ipv4 >> 16) & 0xFF) << '.'
            << ((m_address.ipv4 >> 8) & 0xFF) << '.'
            << (m_address.ipv4 & 0xFF)
            << std::endl;
  } 
  else if (m_family == AddressFamily::IPv6) {
    std::cout << "address: 0x";
    for (size_t i = 0; i < std::size(m_address.ipv6); i++) {
      std::cout << std::setw(8) << std::setfill('0') << m_address.ipv6[i];
    }
    std::cout << " : ";
    for (size_t i = 0; i < std::size(m_address.ipv6); ++i) {
      uint32_t word = m_address.ipv6[i];
      std::cout << std::hex << std::setw(4) << std::setfill('0') << ((word >> 16) & 0xFFFF) << ':'
                << std::setw(4) << std::setfill('0') << (word & 0xFFFF);
    }
    std::cout << std::endl;
  }
}

} // namespace stun
