#include "stun_library/stun_message.hpp"
#include "stun_library/error_code_attribute.hpp"
#include "stun_library/finger_print_attribute.hpp"
#include "stun_library/mapped_address_attribute.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"

#include "test_helpers.hpp"

#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace stun {
namespace {

constexpr uint16_t kMappedAddressPort = 0x1234;
constexpr uint16_t kAlternateMappedAddressPort = 0x5678;
constexpr uint16_t kXorMappedAddressPort = 0x2345;
constexpr uint16_t kXorMappedAddressIpv6Port = 0x3456;
constexpr uint32_t kMappedIpv4Address = 0xC0000201;
constexpr uint32_t kXorMappedIpv4Address = 0xCB007105;
constexpr std::array<uint32_t, 4> kIpv6Address{0x20010DB8, 0x00000000,
                                               0x00000000, 0x00000001};

TEST(StunHeader, SerializesAllBindingClasses) {
  const std::array<std::pair<StunClass, uint16_t>, 4> cases{{
      {StunClass::request, 0x0001},
      {StunClass::indication, 0x0011},
      {StunClass::success_response, 0x0101},
      {StunClass::error_response, 0x0111},
  }};

  for (const auto &[cclass, expected_type] : cases) {
    std::array<uint8_t, kStunHeaderSize> buffer{};
    StunHeader header = test::make_header(cclass);

    EXPECT_EQ(header.serialize(buffer), kStunHeaderSize);

    uint16_t raw_type{};
    std::memcpy(&raw_type, buffer.data(), sizeof(raw_type));
    EXPECT_EQ(ntohs(raw_type), expected_type);

    const StunMessage decoded = StunMessage::deserialize(buffer);
    EXPECT_EQ(decoded.header.method, StunMethod::binding);
    EXPECT_EQ(decoded.header.cclass, cclass);
    EXPECT_EQ(decoded.header.magic_cookie, kMagicCookie);
    EXPECT_EQ(decoded.attributes.size(), 0);
  }
}

TEST(StunHeader, ThrowsOnInvalidInput) {
  std::array<uint8_t, kStunHeaderSize - 1> too_small{};
  EXPECT_THROW(test::make_header().serialize(too_small), std::runtime_error);

  std::array<uint8_t, kStunHeaderSize> buffer{};
  StunHeader unsupported = test::make_header();
  unsupported.method = static_cast<StunMethod>(0x222);
  EXPECT_THROW(unsupported.serialize(buffer), std::runtime_error);
}

TEST(StunMessage, RejectsMalformedMessages) {
  std::array<uint8_t, kStunHeaderSize - 1> too_small{};
  EXPECT_THROW(StunMessage::deserialize(too_small), std::runtime_error);

  std::array<uint8_t, kStunHeaderSize> length_mismatch{};
  StunHeader header = test::make_header();
  header.message_length = 4;
  header.serialize(length_mismatch);
  EXPECT_THROW(StunMessage::deserialize(length_mismatch), std::runtime_error);
}

TEST(StunMessage, CreatesEmptyBindingRequestWithMagicCookie) {
  const StunMessage request = create_stun_request();

  EXPECT_EQ(request.header.method, StunMethod::binding);
  EXPECT_EQ(request.header.cclass, StunClass::request);
  EXPECT_EQ(request.header.message_length, 0);
  EXPECT_EQ(request.header.magic_cookie, kMagicCookie);
  EXPECT_TRUE(request.attributes.empty());
}

TEST(StunMessage, ConstructsBindingMessageWithClassAndTransactionId) {
  const uint32_t transaction_id[3]{0x01020304, 0xA0B0C0D0, 0x11223344};

  const StunMessage message{StunClass::success_response, transaction_id};

  EXPECT_EQ(message.header.method, StunMethod::binding);
  EXPECT_EQ(message.header.cclass, StunClass::success_response);
  EXPECT_EQ(message.header.message_length, 0);
  EXPECT_EQ(message.header.magic_cookie, kMagicCookie);
  EXPECT_EQ(message.header.transaction_id[0], transaction_id[0]);
  EXPECT_EQ(message.header.transaction_id[1], transaction_id[1]);
  EXPECT_EQ(message.header.transaction_id[2], transaction_id[2]);
  EXPECT_TRUE(message.attributes.empty());
}

TEST(StunMessage, RoundTripsMessageWithKnownAttributes) {
  StunMessage message{};
  message.header = test::make_header(StunClass::success_response);
  message.attributes.push_back(std::make_unique<MappedAddressAttribute>(
      MappedAddressAttribute::from_ip_address(
          kMappedAddressPort, IpAddress::from_ipv4(kMappedIpv4Address))));

  message.attributes.push_back(std::make_unique<MappedAddressAttribute>(
      MappedAddressAttribute::from_ip_address(
          kAlternateMappedAddressPort, IpAddress::from_ipv6(kIpv6Address))));

  message.attributes.push_back(std::make_unique<XorMappedAddressAttribute>(
      XorMappedAddressAttribute::from_ip_address(
          kXorMappedAddressPort, IpAddress::from_ipv4(kXorMappedIpv4Address),
          kMagicCookie)));

  std::array<uint32_t, 3> transaction_id{message.header.transaction_id[0],
                                         message.header.transaction_id[1],
                                         message.header.transaction_id[2]};
  message.attributes.push_back(std::make_unique<XorMappedAddressAttribute>(
      XorMappedAddressAttribute::from_ip_address(
          kXorMappedAddressIpv6Port, IpAddress::from_ipv6(kIpv6Address),
          kMagicCookie, transaction_id.data())));

  message.attributes.push_back(std::make_unique<ErrorCodeAttribute>(
      ErrorCodeAttribute::create_error(487, "Bad")));

  constexpr std::array<uint8_t, 4> fingerprint_value{0x12, 0x34, 0x56, 0x78};
  auto fingerprint = std::make_unique<FingerPrintAttribute>();
  fingerprint->compute_finger_print(fingerprint_value);
  message.attributes.push_back(std::move(fingerprint));

  std::array<uint8_t, 512> buffer{};
  const size_t size = message.serialize(buffer);

  uint16_t raw_message_length{};
  std::memcpy(&raw_message_length, buffer.data() + 2, sizeof(raw_message_length));
  EXPECT_EQ(ntohs(raw_message_length), size - kStunHeaderSize);

  const StunMessage decoded =
      StunMessage::deserialize(std::span<const uint8_t>{buffer.data(), size});

  EXPECT_EQ(decoded.header.cclass, StunClass::success_response);
  ASSERT_EQ(decoded.attributes.size(), 6);

  EXPECT_EQ(test::as_attribute<MappedAddressAttribute>(decoded.attributes[0])
                .get_ip_address(),
            IpAddress::from_ipv4(kMappedIpv4Address));
  EXPECT_EQ(test::as_attribute<MappedAddressAttribute>(decoded.attributes[1])
                .get_ip_address(),
            IpAddress::from_ipv6(kIpv6Address));
  EXPECT_EQ(test::as_attribute<XorMappedAddressAttribute>(decoded.attributes[2])
                .get_ip_address(),
            IpAddress::from_ipv4(kXorMappedIpv4Address));
  EXPECT_EQ(test::as_attribute<XorMappedAddressAttribute>(decoded.attributes[3])
                .get_ip_address(),
            IpAddress::from_ipv6(kIpv6Address));
  EXPECT_EQ(test::as_attribute<ErrorCodeAttribute>(decoded.attributes[4]).get_length(),
            7);
  EXPECT_EQ(test::as_attribute<FingerPrintAttribute>(decoded.attributes[5]).get_length(),
            fingerprint_value.size());
}

TEST(StunMessage, IgnoresUnknownAttributes) {
  const std::array<uint8_t, 1> value{0xAA};
  const auto unknown =
      test::make_attribute(static_cast<AttributeTypeId>(0x7777), value);
  const auto message_bytes =
      test::make_message_bytes(test::make_header(), unknown);

  const StunMessage decoded = StunMessage::deserialize(message_bytes);

  EXPECT_TRUE(decoded.attributes.empty());
}

TEST(StunMessage, PrintsHeaderAndAttributes) {
  StunMessage message{};
  message.header = test::make_header(StunClass::error_response);
  message.attributes.push_back(std::make_unique<MappedAddressAttribute>(
      MappedAddressAttribute::from_ip_address(
          kMappedAddressPort, IpAddress::from_ipv4(kMappedIpv4Address))));

  message.attributes.push_back(std::make_unique<MappedAddressAttribute>(
      MappedAddressAttribute::from_ip_address(
          kAlternateMappedAddressPort, IpAddress::from_ipv6(kIpv6Address))));
  message.attributes.push_back(std::make_unique<XorMappedAddressAttribute>(
      XorMappedAddressAttribute::from_ip_address(
          kXorMappedAddressPort, IpAddress::from_ipv4(kXorMappedIpv4Address),
          kMagicCookie)));

  std::array<uint32_t, 3> transaction_id{message.header.transaction_id[0],
                                         message.header.transaction_id[1],
                                         message.header.transaction_id[2]};
  message.attributes.push_back(std::make_unique<XorMappedAddressAttribute>(
      XorMappedAddressAttribute::from_ip_address(
          kXorMappedAddressIpv6Port, IpAddress::from_ipv6(kIpv6Address),
          kMagicCookie, transaction_id.data())));
  message.attributes.push_back(std::make_unique<ErrorCodeAttribute>(
      ErrorCodeAttribute::create_error(404, "Nope")));
  auto fingerprint = std::make_unique<FingerPrintAttribute>();
  fingerprint->compute_finger_print(std::array<uint8_t, 1>{0});
  message.attributes.push_back(std::move(fingerprint));

  testing::internal::CaptureStdout();
  message.print();
  const std::string output = testing::internal::GetCapturedStdout();

  EXPECT_NE(output.find("HEADER"), std::string::npos);
  EXPECT_NE(output.find("error_response"), std::string::npos);
  EXPECT_NE(output.find("MappingAddress"), std::string::npos);
  EXPECT_NE(output.find("XorMappingAddress"), std::string::npos);
  EXPECT_NE(output.find("Error code: 404"), std::string::npos);
  EXPECT_NE(output.find("Fingerprint"), std::string::npos);
}

TEST(StunMessage, PrintsEnumNamesAndFallbacks) {
  std::ostringstream method;
  method << static_cast<StunMethod>(0x222);
  EXPECT_EQ(method.str(), "unknown");

  std::ostringstream cclass;
  cclass << StunClass::request << ' ' << StunClass::indication << ' '
         << StunClass::success_response << ' ' << StunClass::error_response
         << ' ' << static_cast<StunClass>(0x7);
  EXPECT_EQ(cclass.str(),
            "request indication success_response error_response unknown");
}

} // namespace
} // namespace stun
