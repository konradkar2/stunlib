#include "stun_message/finger_print_attribute.hpp"

#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

namespace stun {
namespace {

TEST(FingerPrintAttribute, ComputesAndSerializes) {
  const std::array<uint8_t, 8> message{0x01, 0x02, 0x03, 0x04,
                                       0x05, 0x06, 0x07, 0x08};
  FingerPrintAttribute attr;
  const uint32_t fp = attr.compute_finger_print(message);

  std::array<uint8_t, 4> buffer{};
  const size_t size = attr.serialize(buffer);

  uint32_t net = htonl(fp);
  std::array<uint8_t, 4> expected;
  std::memcpy(expected.data(), &net, 4);

  EXPECT_EQ(attr.get_type(), AttributeTypeId::FingerPrint);
  EXPECT_EQ(attr.get_length(), 4);
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
}

TEST(FingerPrintAttribute, DeserializesAndPrintsValue) {
  constexpr std::array<uint8_t, 4> serialized{0x12, 0x34, 0x56, 0x78};
  FingerPrintAttribute attr;

  attr.deserialize(serialized);

  std::array<uint8_t, 4> buffer{};
  EXPECT_EQ(attr.serialize(buffer), buffer.size());
  EXPECT_EQ(buffer, serialized);

  testing::internal::CaptureStdout();
  attr.print_value();
  const std::string output = testing::internal::GetCapturedStdout();
  EXPECT_NE(output.find("Fingerprint"), std::string::npos);
}

} // namespace
} // namespace stun
