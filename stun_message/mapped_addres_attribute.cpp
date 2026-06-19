#include "mapped_address_attribute.hpp"

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

AttributeTypeId MappedAddressAttribute::get_type() const {
  return AttributeTypeId::MappingAddress;
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

void MappedAddressAttribute::print_value() const {
  //TODO
}

} // namespace stun
