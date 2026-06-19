#include "xor_mapped_address_attribute.hpp"
#include <iostream>
#include <arpa/inet.h>

namespace stun {

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

AttributeTypeId XorMappedAddressAttribute::get_type() const {
  return AttributeTypeId::XorMappingAddress;
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
    //TODO IPv6
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
