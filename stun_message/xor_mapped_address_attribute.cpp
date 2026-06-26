#include "xor_mapped_address_attribute.hpp"
#include <iostream>

namespace stun {

AddressFamily XorMappedAddressAttribute::get_family() const{
    return m_family;
}

uint32_t XorMappedAddressAttribute::get_ipv4_address() const {
  assert(m_family == AddressFamily::IPv4);
  return m_xaddress.ipv4 ^ m_magic_cookie;
}


std::array<uint32_t,4> XorMappedAddressAttribute::get_ipv6_address() const {
  assert(m_family == AddressFamily::IPv6);
  return {m_xaddress.ipv6[0] ^ m_magic_cookie, m_xaddress.ipv6[1] ^ m_transaction_id[0],
            m_xaddress.ipv6[2] ^ m_transaction_id[1], m_xaddress.ipv6[3] ^ m_transaction_id[2]};
}

XorMappedAddressAttribute XorMappedAddressAttribute::create_v4(uint16_t port, uint32_t address,
                                          uint32_t magic_cookie) {
  XorMappedAddressAttribute ret{};

  ret.m_magic_cookie = magic_cookie;
  ret.m_family = AddressFamily::IPv4;
  ret.m_xport = port ^ static_cast<uint16_t>(magic_cookie >> 16);
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

void XorMappedAddressAttribute::set_magic_cookie(const uint32_t magic_cookie) {
  m_magic_cookie = magic_cookie;
}

void XorMappedAddressAttribute::set_transaction_id(const uint32_t transaction_id[3]) {
  for (size_t i = 0; i < 3; i++) {
    m_transaction_id[i] = transaction_id[i];
  }
}

uint16_t XorMappedAddressAttribute::get_length() const {
  return 4 + (get_family() == AddressFamily::IPv4 ? sizeof(AddressFamily::IPv4) : sizeof(AddressFamily::IPv6));
}

void XorMappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  std::memcpy(&m_family, source.data() + 1, 1);
  uint16_t xport_raw;
  std::memcpy(&xport_raw, source.data() + 2, 2);
  m_xport = ntohs(xport_raw);
  if (m_family == AddressFamily::IPv4) {
    uint32_t addres_raw;
    std::memcpy(&addres_raw, source.data() + 4, 4);
    m_xaddress.ipv4 = ntohl(addres_raw);
  }
  else if (m_family == AddressFamily::IPv6) {
    uint32_t address_raw[4] = {};
    std::memcpy(&address_raw, source.data() + 4, 16);
    for (size_t i = 0; i < sizeof(m_xaddress.ipv6); ++i) {
      m_xaddress.ipv6[i] = ntohl(address_raw[i]);
    }
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
  } 
  else if (m_family == AddressFamily::IPv6) {
    uint32_t address[4] = {};
    for (size_t i = 0; i < sizeof(m_xaddress.ipv6); ++i) {
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
  } 
  else if (m_family == AddressFamily::IPv6) {
    std::cout << "IPv6\n";
  }

  std::cout << "xport: 0x" << m_xport << " (decoded) : 0x" << (m_xport ^ static_cast<uint16_t>(m_magic_cookie >> 16)) << std::endl;

  if (m_family == AddressFamily::IPv4) {
    std::cout << "xaddress (XOR): 0x" << std::setw(8) << std::setfill('0') << std::hex << m_xaddress.ipv4 << std::dec;
    uint32_t addr = get_ipv4_address();
    std::cout << " (decoded): "
              << ((addr >> 24) & 0xFF) << '.'
              << ((addr >> 16) & 0xFF) << '.'
              << ((addr >> 8) & 0xFF) << '.'
              << (addr & 0xFF)
              << std::endl;
  } 
  else if (m_family == AddressFamily::IPv6) {
    auto ipv6_decoded = get_ipv6_address();
    std::cout << "xaddress (XOR): 0x";
    for (size_t i = 0; i < sizeof(m_xaddress.ipv6); i++) {
      std::cout << std::setw(8) << std::setfill('0') << std::hex << m_xaddress.ipv6[i] << std::dec;
    }
    std::cout << " (decoded): ";
    for (size_t i = 0; i < ipv6_decoded.size(); ++i) {
      uint32_t word = ipv6_decoded[i];
      std::cout << std::hex << std::setw(4) << std::setfill('0') << ((word >> 16) & 0xFFFF) << ':'
                << std::setw(4) << std::setfill('0') << (word & 0xFFFF);
      if (i + 1 < ipv6_decoded.size()) {
        std::cout << ':';
      }
    }
    std::cout << std::dec << std::endl;
  }
}

} // namespace stun
