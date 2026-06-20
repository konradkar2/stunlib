#include "stun_attribute.hpp"
#include "stun_message.hpp"
#include "mapped_address_attribute.hpp"
#include "xor_mapped_address_attribute.hpp"
#include "error_code_attribute.hpp"

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

namespace stun {
namespace {

TEST(MappedAddressAttribute, SerializesIpv4Value) {
  auto attr = MappedAddressAttribute::create_v4(0x1234, 0xC0000201);
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  const std::array<uint8_t, 8> expected{
      0x00, 0x01, 0x12, 0x34, 0xC0, 0x00, 0x02, 0x01};
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
}

TEST(MappedAddressAttribute, SerializesIpv6Value) {
  std::array<uint32_t, 4> address{0x20010DB8, 0x00000000, 0x00000000,
                                  0x00000001};
  auto attr = MappedAddressAttribute::create_v6(0x1234, address.data());
  std::array<uint8_t, 20> buffer{};

  const size_t size = attr.serialize(buffer);

  const std::array<uint8_t, 20> expected{
      0x00, 0x02, 0x12, 0x34, 0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
}

TEST(XorMappedAddressAttribute, SerializesIpv4Value) {
  auto attr = XorMappedAddressAttribute::create_v4(0x1234, 0xC0000201,
                                                  kMagicCookie);
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  // src:    00 01 12 34 C0 00 02 01
  // mask:   00 00 21 12 21 12 A4 42
  // result: 00 01 33 26 E1 12 A6 43
  const std::array<uint8_t, 8> expected{
      0x00, 0x01, 0x33, 0x26, 0xE1, 0x12, 0xA6, 0x43};
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
}

TEST(ErrorCodeAttribute, SerializesValue) {
  auto attr = ErrorCodeAttribute::create_error(404, "Test");
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  const std::array<uint8_t, 8> expected{0x00, 0x00, 0x04, 0x04, 'T', 'e', 's', 't'};
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
}

} // namespace
} // namespace stun
