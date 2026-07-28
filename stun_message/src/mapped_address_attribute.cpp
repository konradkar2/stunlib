#include "stun_message/mapped_address_attribute.hpp"
#include <iostream>
#include <cstring>
#include <iterator>

namespace stun {
namespace {

constexpr size_t kReservedOffset = 0;
constexpr size_t kFamilyOffset = 1;
constexpr size_t kPortOffset = 2;
constexpr size_t kAddressOffset = 4;
constexpr size_t kMappedAddressHeaderLength = kAddressOffset;
constexpr size_t kIpv4AddressLength = sizeof(uint32_t);
constexpr size_t kIpv6AddressLength = sizeof(IpAddress::Ipv6Address);
constexpr uint8_t kReservedValue = 0;

} // namespace

MappedAddressAttribute
MappedAddressAttribute::from_ip_address(uint16_t port, IpAddress address) {
  MappedAddressAttribute ret{};

  ret.m_port = port;
  ret.m_address = address;

  return ret;
}

uint16_t MappedAddressAttribute::get_length() const {
  return static_cast<uint16_t>(
      kMappedAddressHeaderLength +
      (get_family() == AddressFamily::IPv4 ? kIpv4AddressLength
                                           : kIpv6AddressLength));
}

void MappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  uint8_t family;
  std::memcpy(&family, source.data() + kFamilyOffset, sizeof(family));
  const auto address_family = static_cast<AddressFamily>(family);

  uint16_t port_raw;
  std::memcpy(&port_raw, source.data() + kPortOffset, sizeof(port_raw));
  m_port = ntohs(port_raw);

  if (address_family == AddressFamily::IPv4) {
    uint32_t address_raw;
    std::memcpy(&address_raw, source.data() + kAddressOffset,
                sizeof(address_raw));
    m_address = IpAddress::from_ipv4(ntohl(address_raw));
  }
  else if (address_family == AddressFamily::IPv6) {
    IpAddress::Ipv6Address address_raw{};
    std::memcpy(address_raw.data(), source.data() + kAddressOffset,
                kIpv6AddressLength);

    IpAddress::Ipv6Address address{};
    for (size_t i = 0; i < address.size(); ++i) {
      address[i] = ntohl(address_raw[i]);
    }
    m_address = IpAddress::from_ipv6(address);
  }
}

size_t MappedAddressAttribute::serialize(std::span<uint8_t> target) const {
  uint8_t reserved = kReservedValue;
  std::memcpy(target.data() + kReservedOffset, &reserved, sizeof(reserved));

  uint8_t family = static_cast<uint8_t>(get_family());
  std::memcpy(target.data() + kFamilyOffset, &family, sizeof(family));

  uint16_t port = htons(m_port);
  std::memcpy(target.data() + kPortOffset, &port, sizeof(port));

  if (get_family() == AddressFamily::IPv4) {
    uint32_t address = htonl(m_address.ipv4());
    std::memcpy(target.data() + kAddressOffset, &address, sizeof(address));
    return kMappedAddressHeaderLength + kIpv4AddressLength;
  }

  const auto ipv6_address = m_address.ipv6();
  IpAddress::Ipv6Address address{};
  for (size_t i = 0; i < ipv6_address.size(); ++i) {
    address[i] = htonl(ipv6_address[i]);
  }
  std::memcpy(target.data() + kAddressOffset, address.data(),
              kIpv6AddressLength);
  return kMappedAddressHeaderLength + kIpv6AddressLength;
}

AddressFamily MappedAddressAttribute::get_family() const{
  return m_address.is_ipv4() ? AddressFamily::IPv4 : AddressFamily::IPv6;
}

uint16_t MappedAddressAttribute::get_port() const {
  return m_port;
}

IpAddress MappedAddressAttribute::get_ip_address() const {
  return m_address;
}

void MappedAddressAttribute::print_value() const {
  std::cout << "family: ";
  if (get_family() == AddressFamily::IPv4) {
    std::cout << "IPv4\n";
  } 
  else if (get_family() == AddressFamily::IPv6) {
    std::cout << "IPv6\n";
  }

  std::cout << "xport: 0x" << m_port << '\n';

  if (get_family() == AddressFamily::IPv4) {
    const uint32_t address = m_address.ipv4();
    std::cout << "address: 0x" << std::setw(8) << std::setfill('0')
              << address << " : ";
    std::cout << " (decoded): " << m_address << std::endl;
  } 
  else if (get_family() == AddressFamily::IPv6) {
    const auto address = m_address.ipv6();
    std::cout << "address: 0x";
    for (const uint32_t word : address) {
      std::cout << std::setw(8) << std::setfill('0') << word;
    }
    std::cout << " : " << m_address;
    std::cout << std::endl;
  }
}

} // namespace stun
