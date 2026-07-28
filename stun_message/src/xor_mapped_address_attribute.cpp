#include "stun_message/xor_mapped_address_attribute.hpp"
#include <iostream>
#include <cstring>
#include <iterator>
#include <stdexcept>
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
constexpr size_t kMagicCookieIpv6WordIndex = 0;
constexpr size_t kFirstTransactionIdIpv6WordIndex = 1;
constexpr uint16_t kMagicCookiePortMaskShift = 16;

uint16_t xor_port(uint16_t port, uint32_t magic_cookie) {
  return port ^ static_cast<uint16_t>(magic_cookie >> kMagicCookiePortMaskShift);
}

} // namespace

AddressFamily XorMappedAddressAttribute::get_family() const{
    return m_xaddress.is_ipv4() ? AddressFamily::IPv4 : AddressFamily::IPv6;
}

uint16_t XorMappedAddressAttribute::get_port() const {
  return xor_port(m_xport, m_magic_cookie);
}

IpAddress XorMappedAddressAttribute::get_ip_address() const {
  if (get_family() == AddressFamily::IPv4) {
    return IpAddress::from_ipv4(m_xaddress.ipv4() ^ m_magic_cookie);
  }

  const auto xaddress = m_xaddress.ipv6();
  IpAddress::Ipv6Address address{};
  address[kMagicCookieIpv6WordIndex] =
      xaddress[kMagicCookieIpv6WordIndex] ^ m_magic_cookie;
  for (size_t i = kFirstTransactionIdIpv6WordIndex; i < address.size(); ++i) {
    address[i] = xaddress[i] ^ m_transaction_id[i - 1];
  }
  return IpAddress::from_ipv6(address);
}

XorMappedAddressAttribute XorMappedAddressAttribute::from_ip_address(
    uint16_t port, IpAddress address, uint32_t magic_cookie) {
  if (address.is_ipv6()) {
    throw std::invalid_argument(
        "transaction id is required for IPv6 XOR-MAPPED-ADDRESS");
  }

  XorMappedAddressAttribute ret{};

  ret.m_magic_cookie = magic_cookie;
  ret.m_xport = xor_port(port, magic_cookie);
  ret.m_xaddress = IpAddress::from_ipv4(address.ipv4() ^ magic_cookie);
                                            
  return ret;
}

XorMappedAddressAttribute XorMappedAddressAttribute::from_ip_address(
    uint16_t port, IpAddress address, uint32_t magic_cookie,
    const uint32_t transaction_id[3]) {
  if (address.is_ipv4()) {
    return from_ip_address(port, address, magic_cookie);
  }

  XorMappedAddressAttribute ret{};

  ret.m_magic_cookie = magic_cookie;
  memcpy(ret.m_transaction_id, transaction_id, sizeof(ret.m_transaction_id));
  ret.m_xport = xor_port(port, magic_cookie);

  const auto ipv6_address = address.ipv6();
  IpAddress::Ipv6Address xaddress{};
  xaddress[kMagicCookieIpv6WordIndex] =
      ipv6_address[kMagicCookieIpv6WordIndex] ^ magic_cookie;
  for (size_t i = kFirstTransactionIdIpv6WordIndex; i < xaddress.size(); i++) {
    xaddress[i] = ipv6_address[i] ^ transaction_id[i - 1];
  }
  ret.m_xaddress = IpAddress::from_ipv6(xaddress);
                                  
                                            
  return ret;
}

void XorMappedAddressAttribute::set_magic_cookie(const uint32_t magic_cookie) {
  m_magic_cookie = magic_cookie;
}

void XorMappedAddressAttribute::set_transaction_id(const uint32_t transaction_id[3]) {
  for (size_t i = 0; i < std::size(m_transaction_id); i++) {
    m_transaction_id[i] = transaction_id[i];
  }
}

uint16_t XorMappedAddressAttribute::get_length() const {
  return static_cast<uint16_t>(
      kMappedAddressHeaderLength +
      (get_family() == AddressFamily::IPv4 ? kIpv4AddressLength
                                           : kIpv6AddressLength));
}

void XorMappedAddressAttribute::deserialize(std::span<const uint8_t> source) {
  uint8_t family;
  std::memcpy(&family, source.data() + kFamilyOffset, sizeof(family));
  const auto address_family = static_cast<AddressFamily>(family);

  uint16_t xport_raw;
  std::memcpy(&xport_raw, source.data() + kPortOffset, sizeof(xport_raw));
  m_xport = ntohs(xport_raw);

  if (address_family == AddressFamily::IPv4) {
    uint32_t address_raw;
    std::memcpy(&address_raw, source.data() + kAddressOffset,
                sizeof(address_raw));
    m_xaddress = IpAddress::from_ipv4(ntohl(address_raw));
  }
  else if (address_family == AddressFamily::IPv6) {
    IpAddress::Ipv6Address address_raw{};
    std::memcpy(address_raw.data(), source.data() + kAddressOffset,
                kIpv6AddressLength);

    IpAddress::Ipv6Address xaddress{};
    for (size_t i = 0; i < xaddress.size(); ++i) {
      xaddress[i] = ntohl(address_raw[i]);
    }
    m_xaddress = IpAddress::from_ipv6(xaddress);
  }
}

size_t XorMappedAddressAttribute::serialize(std::span<uint8_t> target) const {
  uint8_t reserved = kReservedValue;
  std::memcpy(target.data() + kReservedOffset, &reserved, sizeof(reserved));

  uint8_t family = static_cast<uint8_t>(get_family());
  std::memcpy(target.data() + kFamilyOffset, &family, sizeof(family));

  uint16_t port = htons(m_xport);
  std::memcpy(target.data() + kPortOffset, &port, sizeof(port));

  if (get_family() == AddressFamily::IPv4) {
    uint32_t address = htonl(m_xaddress.ipv4());
    std::memcpy(target.data() + kAddressOffset, &address, sizeof(address));
    return kMappedAddressHeaderLength + kIpv4AddressLength;
  }

  const auto xaddress = m_xaddress.ipv6();
  IpAddress::Ipv6Address address{};
  for (size_t i = 0; i < xaddress.size(); ++i) {
    address[i] = htonl(xaddress[i]);
  }
  std::memcpy(target.data() + kAddressOffset, address.data(),
              kIpv6AddressLength);
  return kMappedAddressHeaderLength + kIpv6AddressLength;
}

void XorMappedAddressAttribute::print_value() const {
  std::cout << "family: ";
  if (get_family() == AddressFamily::IPv4) {
    std::cout << "IPv4\n";
  } 
  else if (get_family() == AddressFamily::IPv6) {
    std::cout << "IPv6\n";
  }

  std::cout << "xport: 0x" << m_xport << " (decoded) : 0x"
            << xor_port(m_xport, m_magic_cookie) << std::endl;

  if (get_family() == AddressFamily::IPv4) {
    std::cout << "xaddress (XOR): 0x" << std::setw(8) << std::setfill('0')
              << std::hex << m_xaddress.ipv4() << std::dec;
    std::cout << " (decoded): " << get_ip_address() << std::endl;
  } 
  else if (get_family() == AddressFamily::IPv6) {
    const auto xaddress = m_xaddress.ipv6();
    std::cout << "xaddress (XOR): 0x";
    for (const uint32_t word : xaddress) {
      std::cout << std::setw(8) << std::setfill('0') << std::hex << word
                << std::dec;
    }
    std::cout << " (decoded): " << get_ip_address() << std::endl;
  }
}

} // namespace stun
