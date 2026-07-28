#include "stun_library/ip_address.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <variant>

namespace stun {
namespace {

constexpr uint32_t kIpv4Address = 0xC0000201; // 192.0.2.1
constexpr IpAddress::Ipv6Address kIpv6Address{
    0x20010DB8, 0x00000000, 0x00000000, 0x00000001};

TEST(IpAddress, StoresIpv4Address) {
  const auto address = IpAddress::from_ipv4(kIpv4Address);

  EXPECT_TRUE(address.is_ipv4());
  EXPECT_FALSE(address.is_ipv6());
  EXPECT_EQ(address.ipv4(), kIpv4Address);
  EXPECT_EQ(address.to_string(), "192.0.2.1");
}

TEST(IpAddress, StoresIpv6Address) {
  const auto address = IpAddress::from_ipv6(kIpv6Address);

  EXPECT_FALSE(address.is_ipv4());
  EXPECT_TRUE(address.is_ipv6());
  EXPECT_EQ(address.ipv6(), kIpv6Address);
  EXPECT_EQ(address.to_string(), "2001:0db8:0000:0000:0000:0000:0000:0001");
}

TEST(IpAddress, ComparesAddresses) {
  EXPECT_EQ(IpAddress::from_ipv4(kIpv4Address),
            IpAddress::from_ipv4(kIpv4Address));
  EXPECT_EQ(IpAddress::from_ipv6(kIpv6Address),
            IpAddress::from_ipv6(kIpv6Address));
  EXPECT_NE(IpAddress::from_ipv4(kIpv4Address),
            IpAddress::from_ipv6(kIpv6Address));
}

TEST(IpAddress, ThrowsWhenReadingWrongAddressFamily) {
  const auto ipv4 = IpAddress::from_ipv4(kIpv4Address);
  const auto ipv6 = IpAddress::from_ipv6(kIpv6Address);

  EXPECT_THROW(ipv4.ipv6(), std::bad_variant_access);
  EXPECT_THROW(ipv6.ipv4(), std::bad_variant_access);
}

TEST(IpAddress, PrintsAddress) {
  std::ostringstream output;

  output << IpAddress::from_ipv4(kIpv4Address);

  EXPECT_EQ(output.str(), "192.0.2.1");
}

} // namespace
} // namespace stun
