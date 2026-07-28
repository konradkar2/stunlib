#include "stun_message/xor_mapped_address_attribute.hpp"
#include "stun_message/stun_message.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>

namespace stun {
namespace {

constexpr uint16_t kPort = 0x1234;
constexpr uint32_t kIpv4Address = 0xC0000201; // 192.0.2.1
constexpr std::array<uint32_t, 4> kIpv6Address{0x20010DB8, 0x00000000,
                                               0x00000000, 0x00000001};
constexpr std::array<uint32_t, 3> kTransactionId{0x01020304, 0xA0B0C0D0,
                                                 0x11223344};
constexpr std::array<uint8_t, 8> kSerializedIpv4Value{
    0x00, 0x01, 0x33, 0x26, 0xE1, 0x12, 0xA6, 0x43};
constexpr std::array<uint8_t, 20> kSerializedIpv6Value{
    0x00, 0x02, 0x33, 0x26, 0x01, 0x13, 0xA9, 0xFA, 0x01, 0x02,
    0x03, 0x04, 0xA0, 0xB0, 0xC0, 0xD0, 0x11, 0x22, 0x33, 0x45};

TEST(XorMappedAddressAttribute, SerializesIpv4Value) {
  auto attr = XorMappedAddressAttribute::from_ip_address(
      kPort, IpAddress::from_ipv4(kIpv4Address), kMagicCookie);
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  EXPECT_EQ(size, kSerializedIpv4Value.size());
  EXPECT_EQ(buffer, kSerializedIpv4Value);
  EXPECT_EQ(attr.get_type(), AttributeTypeId::XorMappingAddress);
  EXPECT_EQ(attr.get_family(), AddressFamily::IPv4);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 8);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv4(kIpv4Address));
}

TEST(XorMappedAddressAttribute, SerializesIpv6Value) {
  auto attr = XorMappedAddressAttribute::from_ip_address(
      kPort, IpAddress::from_ipv6(kIpv6Address), kMagicCookie,
      kTransactionId.data());
  std::array<uint8_t, 20> buffer{};

  const size_t size = attr.serialize(buffer);

  EXPECT_EQ(size, kSerializedIpv6Value.size());
  EXPECT_EQ(buffer, kSerializedIpv6Value);
  EXPECT_EQ(attr.get_family(), AddressFamily::IPv6);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 20);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv6(kIpv6Address));
}

TEST(XorMappedAddressAttribute, DeserializesIpv4Value) {
  XorMappedAddressAttribute attr;

  attr.deserialize(kSerializedIpv4Value);
  attr.set_magic_cookie(kMagicCookie);

  EXPECT_EQ(attr.get_family(), AddressFamily::IPv4);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 8);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv4(kIpv4Address));
}

TEST(XorMappedAddressAttribute, DeserializesIpv6Value) {
  XorMappedAddressAttribute attr;

  attr.deserialize(kSerializedIpv6Value);
  attr.set_magic_cookie(kMagicCookie);
  attr.set_transaction_id(kTransactionId.data());

  EXPECT_EQ(attr.get_family(), AddressFamily::IPv6);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 20);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv6(kIpv6Address));
}

TEST(XorMappedAddressAttribute, PrintsIpv4AndIpv6Values) {
  auto ipv4 = XorMappedAddressAttribute::from_ip_address(
      kPort, IpAddress::from_ipv4(kIpv4Address), kMagicCookie);
  testing::internal::CaptureStdout();
  ipv4.print_value();
  const std::string ipv4_output = testing::internal::GetCapturedStdout();
  EXPECT_NE(ipv4_output.find("IPv4"), std::string::npos);
  EXPECT_NE(ipv4_output.find("192.0.2.1"), std::string::npos);

  auto ipv6 = XorMappedAddressAttribute::from_ip_address(
      0x5678, IpAddress::from_ipv6(kIpv6Address), kMagicCookie,
      kTransactionId.data());
  testing::internal::CaptureStdout();
  ipv6.print_value();
  const std::string ipv6_output = testing::internal::GetCapturedStdout();
  EXPECT_NE(ipv6_output.find("IPv6"), std::string::npos);
  EXPECT_NE(ipv6_output.find("2001:0db8"), std::string::npos);
}

} // namespace
} // namespace stun
