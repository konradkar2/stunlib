#include <stdint.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>

constexpr uint32_t kBufferSize = 1024 * 64;
constexpr uint32_t kMagicCookie = 0x2112A442;
constexpr uint32_t kStunHeaderSize = 20;

enum class StunMethod {
  binding = 1,
};

enum class StunClass {
  request = 0,
  indication = 1,
  success_response = 2,
  error_response = 3
};

uint16_t create_stun_message_type(StunMethod method,
                                  StunClass cls) {
  if (method != StunMethod::binding) {
    throw std::runtime_error("unimplemented method");
  }
  const uint16_t m = static_cast<uint16_t>(method);
  const uint16_t c = static_cast<uint16_t>(cls);
  uint16_t message_type =
    (m & 0x000F) |              // M0-M3 -> bits 0..3
    ((m & 0x0070) << 1) |       // M4-M6 -> bits 5..7
    ((m & 0x0F80) << 2) |       // M7-M11 -> bits 9..13
    ((c & 0x02) << 7) |         // C1 -> bit 8
    ((c & 0x01) << 4);          // C0 -> bit 4

  return message_type;
}

struct stun_header {
  uint16_t message_type;
  uint16_t message_length;
  uint32_t magic_cookie;
  uint32_t transaction_id[3];
};
static_assert(sizeof(stun_header) == kStunHeaderSize);

stun_header create_stun_request() {
  stun_header header{};

  uint16_t message_type = create_stun_message_type(StunMethod::binding, StunClass::request);
  header.message_type = htons(message_type);
  header.message_length = 0;
  header.magic_cookie = htonl(kMagicCookie);
  header.transaction_id[0] = 1;

  return header;
}

void print_stun_message(const stun_header &header);
void convert_bytes_to_stun_message();