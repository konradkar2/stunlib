#include "stun_message/mapped_address_attribute.hpp"

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
constexpr std::array<uint8_t, 8> kSerializedIpv4Value{
    0x00, 0x01, 0x12, 0x34, 0xC0, 0x00, 0x02, 0x01};
constexpr std::array<uint8_t, 20> kSerializedIpv6Value{
    0x00, 0x02, 0x12, 0x34, 0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

TEST(MappedAddressAttribute, SerializesIpv4Value) {
  auto attr = MappedAddressAttribute::from_ip_address(kPort, IpAddress::from_ipv4(kIpv4Address));
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  EXPECT_EQ(size, kSerializedIpv4Value.size());
  EXPECT_EQ(buffer, kSerializedIpv4Value);
  EXPECT_EQ(attr.get_type(), AttributeTypeId::MappingAddress);
  EXPECT_EQ(attr.get_family(), AddressFamily::IPv4);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 8);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv4(kIpv4Address));
}

TEST(MappedAddressAttribute, SerializesIpv6Value) {
  auto attr = MappedAddressAttribute::from_ip_address(kPort, IpAddress::from_ipv6(kIpv6Address));
  std::array<uint8_t, 20> buffer{};

  const size_t size = attr.serialize(buffer);

  EXPECT_EQ(size, kSerializedIpv6Value.size());
  EXPECT_EQ(buffer, kSerializedIpv6Value);
  EXPECT_EQ(attr.get_family(), AddressFamily::IPv6);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 20);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv6(kIpv6Address));
}

TEST(MappedAddressAttribute, DeserializesIpv4Value) {
  MappedAddressAttribute attr;

  attr.deserialize(kSerializedIpv4Value);

  EXPECT_EQ(attr.get_family(), AddressFamily::IPv4);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 8);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv4(kIpv4Address));
}

TEST(MappedAddressAttribute, DeserializesIpv6Value) {
  MappedAddressAttribute attr;

  attr.deserialize(kSerializedIpv6Value);

  EXPECT_EQ(attr.get_family(), AddressFamily::IPv6);
  EXPECT_EQ(attr.get_port(), kPort);
  EXPECT_EQ(attr.get_length(), 20);
  EXPECT_EQ(attr.get_ip_address(), IpAddress::from_ipv6(kIpv6Address));
}

TEST(MappedAddressAttribute, PrintsIpv4AndIpv6Values) {
  auto ipv4 = MappedAddressAttribute::from_ip_address(kPort, IpAddress::from_ipv4(kIpv4Address));
  testing::internal::CaptureStdout();
  ipv4.print_value();
  const std::string ipv4_output = testing::internal::GetCapturedStdout();
  EXPECT_NE(ipv4_output.find("IPv4"), std::string::npos);
  EXPECT_NE(ipv4_output.find("192.0.2.1"), std::string::npos);

  auto ipv6 = MappedAddressAttribute::from_ip_address(0x5678, IpAddress::from_ipv6(kIpv6Address));
  testing::internal::CaptureStdout();
  ipv6.print_value();
  const std::string ipv6_output = testing::internal::GetCapturedStdout();
  EXPECT_NE(ipv6_output.find("IPv6"), std::string::npos);
  EXPECT_NE(ipv6_output.find("address"), std::string::npos);
}

} // namespace
} // namespace stun
