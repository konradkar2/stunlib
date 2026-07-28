#pragma once

#include "stun_message/stun_attribute.hpp"
#include "stun_message/stun_message.hpp"

#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace stun::test {

inline size_t padded(size_t length) {
  return (length + 3) & ~static_cast<size_t>(3);
}

inline void append_u16(std::vector<uint8_t> &target, uint16_t value) {
  const uint16_t net = htons(value);
  const auto *bytes = reinterpret_cast<const uint8_t *>(&net);
  target.insert(target.end(), bytes, bytes + sizeof(net));
}

inline std::vector<uint8_t> make_attribute(AttributeTypeId type,
                                           std::span<const uint8_t> value) {
  std::vector<uint8_t> result;
  append_u16(result, static_cast<uint16_t>(type));
  append_u16(result, static_cast<uint16_t>(value.size()));
  result.insert(result.end(), value.begin(), value.end());
  result.resize(4 + padded(value.size()), 0);
  return result;
}

inline StunHeader make_header(StunClass cclass = StunClass::request) {
  return StunHeader{
      .method = StunMethod::binding,
      .cclass = cclass,
      .message_length = 0,
      .magic_cookie = kMagicCookie,
      .transaction_id = {0x01020304, 0xA0B0C0D0, 0x11223344},
  };
}

inline std::vector<uint8_t> make_message_bytes(const StunHeader &header,
                                               std::span<const uint8_t> attributes) {
  std::vector<uint8_t> result(kStunHeaderSize + attributes.size());
  StunHeader serialized_header = header;
  serialized_header.message_length = static_cast<uint16_t>(attributes.size());
  serialized_header.serialize(result);
  std::copy(attributes.begin(), attributes.end(), result.begin() + kStunHeaderSize);
  return result;
}

template <typename T>
const T &as_attribute(const std::unique_ptr<StunAttribute> &attribute) {
  const auto *typed = dynamic_cast<const T *>(attribute.get());
  EXPECT_NE(typed, nullptr);
  return *typed;
}

} // namespace stun::test
