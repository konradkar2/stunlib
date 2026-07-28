#include "stun_library/ip_address.hpp"

#include <iomanip>
#include <ostream>
#include <sstream>

namespace stun {
namespace {

constexpr uint32_t kByteMask = 0xFF;
constexpr uint32_t kIpv6SegmentMask = 0xFFFF;
constexpr size_t kBitsPerByte = 8;
constexpr size_t kIpv4FirstOctetShift = 3 * kBitsPerByte;
constexpr size_t kIpv4SecondOctetShift = 2 * kBitsPerByte;
constexpr size_t kIpv4ThirdOctetShift = kBitsPerByte;
constexpr size_t kIpv4FourthOctetShift = 0;
constexpr size_t kIpv6HighSegmentShift = 2 * kBitsPerByte;

} // namespace

IpAddress::IpAddress(uint32_t address) : m_address(address) {}

IpAddress::IpAddress(Ipv6Address address) : m_address(address) {}

IpAddress IpAddress::from_ipv4(uint32_t address) {
  return IpAddress{address};
}

IpAddress IpAddress::from_ipv6(Ipv6Address address) {
  return IpAddress{address};
}

bool IpAddress::is_ipv4() const {
  return std::holds_alternative<uint32_t>(m_address);
}

bool IpAddress::is_ipv6() const {
  return std::holds_alternative<Ipv6Address>(m_address);
}

uint32_t IpAddress::ipv4() const {
  return std::get<uint32_t>(m_address);
}

IpAddress::Ipv6Address IpAddress::ipv6() const {
  return std::get<Ipv6Address>(m_address);
}

std::string IpAddress::to_string() const {
  std::ostringstream output;

  if (is_ipv4()) {
    const uint32_t address = ipv4();
    output << ((address >> kIpv4FirstOctetShift) & kByteMask) << '.'
           << ((address >> kIpv4SecondOctetShift) & kByteMask) << '.'
           << ((address >> kIpv4ThirdOctetShift) & kByteMask) << '.'
           << ((address >> kIpv4FourthOctetShift) & kByteMask);
    return output.str();
  }

  const auto address = ipv6();
  output << std::hex << std::setfill('0');
  for (size_t i = 0; i < address.size(); ++i) {
    if (i != 0) {
      output << ':';
    }

    output << std::setw(4)
           << ((address[i] >> kIpv6HighSegmentShift) & kIpv6SegmentMask)
           << ':' << std::setw(4) << (address[i] & kIpv6SegmentMask);
  }

  return output.str();
}

std::ostream &operator<<(std::ostream &os, const IpAddress &address) {
  return os << address.to_string();
}

} // namespace stun
