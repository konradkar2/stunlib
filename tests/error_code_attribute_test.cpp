#include "stun_library/error_code_attribute.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>

namespace stun {
namespace {

constexpr uint16_t kBadRequestErrorCode = 404;
constexpr std::array<uint8_t, 8> kSerializedPaddedRoleConflictError{
    0x00, 0x00, 0x04, 0x57, 'B', 'a', 'd', 0x00};

TEST(ErrorCodeAttribute, SerializesValue) {
  auto attr = ErrorCodeAttribute::create_error(kBadRequestErrorCode, "Test");
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  const std::array<uint8_t, 8> expected{0x00, 0x00, 0x04, 0x04,
                                        'T',  'e',  's',  't'};
  EXPECT_EQ(size, expected.size());
  EXPECT_EQ(buffer, expected);
  EXPECT_EQ(attr.get_type(), AttributeTypeId::ErrorCode);
  EXPECT_EQ(attr.get_length(), 8);
}

TEST(ErrorCodeAttribute, SerializesAndPadsOddLengthReasonPhrase) {
  auto attr = ErrorCodeAttribute::create_error(487, "Bad");
  std::array<uint8_t, 8> buffer{};

  const size_t size = attr.serialize(buffer);

  EXPECT_EQ(size, kSerializedPaddedRoleConflictError.size());
  EXPECT_EQ(buffer, kSerializedPaddedRoleConflictError);
  EXPECT_EQ(attr.get_length(), 7);
}

TEST(ErrorCodeAttribute, DeserializesValue) {
  ErrorCodeAttribute attr;

  attr.deserialize(kSerializedPaddedRoleConflictError);

  EXPECT_EQ(attr.get_type(), AttributeTypeId::ErrorCode);
  EXPECT_EQ(attr.get_length(), kSerializedPaddedRoleConflictError.size());
  std::array<uint8_t, 8> buffer{};
  EXPECT_EQ(attr.serialize(buffer), buffer.size());
  EXPECT_EQ(buffer, kSerializedPaddedRoleConflictError);
}

TEST(ErrorCodeAttribute, PrintsValue) {
  auto attr = ErrorCodeAttribute::create_error(kBadRequestErrorCode, "Nope");

  testing::internal::CaptureStdout();
  attr.print_value();
  const std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("Error code: 404"), std::string::npos);
  EXPECT_NE(output.find("Reason phrase: Nope"), std::string::npos);
}

} // namespace
} // namespace stun
