#pragma once
#include "stun_message/stun_attribute.hpp"
#include <cstdint>
#include <cstring>

#include <netinet/in.h>
#include <span>
#include <memory>
#include <cstdlib>
#include <vector>

namespace stun {

constexpr uint32_t kBufferSize = 1024 * 64;
constexpr uint32_t kMagicCookie = 0x2112A442;
constexpr uint32_t kStunHeaderSize = 20;

enum class StunMethod {
  binding = 1,
};

std::ostream& operator<<(std::ostream& os, StunMethod method);

enum class StunClass {
  request = 0,
  indication = 1,
  success_response = 2,
  error_response = 3
};

std::ostream& operator<<(std::ostream& os, StunClass cclass);

struct StunHeader {
  StunMethod method;
  StunClass cclass;
  uint16_t message_length;
  uint32_t magic_cookie;
  uint32_t transaction_id[3];

  size_t serialize(std::span<uint8_t> target) const;
};

struct StunMessage {
  StunHeader header;
  std::vector<std::unique_ptr<StunAttribute>> attributes;

  static StunMessage deserialize(std::span<const uint8_t> src);
  size_t serialize(std::span<uint8_t> target) const;
  void print();
};

StunMessage create_stun_request();

} // namespace stun
