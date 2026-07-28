#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <variant>

namespace stun {

class IpAddress {
public:
  using Ipv6Address = std::array<uint32_t, 4>;

  static IpAddress from_ipv4(uint32_t address);
  static IpAddress from_ipv6(Ipv6Address address);

  bool is_ipv4() const;
  bool is_ipv6() const;

  uint32_t ipv4() const;
  Ipv6Address ipv6() const;

  std::string to_string() const;

  friend bool operator==(const IpAddress &lhs, const IpAddress &rhs) = default;

private:
  explicit IpAddress(uint32_t address);
  explicit IpAddress(Ipv6Address address);

  std::variant<uint32_t, Ipv6Address> m_address;
};

std::ostream &operator<<(std::ostream &os, const IpAddress &address);

} // namespace stun
