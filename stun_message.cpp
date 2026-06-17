#include "stun_message.hpp"
#include <iostream>

namespace stun {

struct StunHeaderRaw {
  uint16_t type;
  uint16_t message_length;
  uint32_t magic_cookie;
  uint32_t transaction_id[3];
};

static_assert(sizeof(StunHeaderRaw) == kStunHeaderSize);

uint16_t encode_message_type(StunMethod method, StunClass cls) {
  if (method != StunMethod::binding) {
    throw std::runtime_error("unimplemented method");
  }
  const uint16_t m = static_cast<uint16_t>(method);
  const uint16_t c = static_cast<uint16_t>(cls);
  uint16_t message_type = (m & 0x000F) |        // M0-M3 -> bits 0..3
                          ((m & 0x0070) << 1) | // M4-M6 -> bits 5..7
                          ((m & 0x0F80) << 2) | // M7-M11 -> bits 9..13
                          ((c & 0x02) << 7) |   // C1 -> bit 8
                          ((c & 0x01) << 4);    // C0 -> bit 4

  return message_type;
}

std::pair<StunMethod, StunClass> decode_message_type(uint16_t message_type) {
  uint16_t m = (message_type & 0x000F) |        // M0-M3
               ((message_type & 0x00E0) >> 1) | // M4-M6
               ((message_type & 0x3E00) >> 2);  // M7-M11

  uint16_t c = ((message_type & 0x0100) >> 7) | // C1
               ((message_type & 0x0010) >> 4);  // C0

  return {static_cast<StunMethod>(m), static_cast<StunClass>(c)};
}

std::ostream& operator<<(std::ostream& os, StunMethod method) {
  switch (method) {
    case StunMethod::binding: return os << "binding";
  }
  return os << "unknown";
}

std::ostream& operator<<(std::ostream& os, StunClass cclass) {
  switch (cclass) {
    case StunClass::request: return os << "request";
    case StunClass::indication: return os << "indication";
    case StunClass::success_response: return os << "success_response";
    case StunClass::error_response: return os << "error_response";
  }
  return os << "unknown";
}

void StunMessage::print() {
  std::cout << "HEADER\n";
  std::cout << std::hex;
  std::cout << "Message method: 0x" << std::setw(4) << std::setfill('0')
            << static_cast<int>(header.method) << " (" << header.method << ")" << std::endl;
  std::cout << "Message class: 0x" << std::setw(4) << std::setfill('0')
            << static_cast<int>(header.cclass) << " (" << header.cclass << ")" << std::endl;
  std::cout << "Message length: 0x" << std::setw(4) << std::setfill('0')
            << header.message_length << std::endl;
  std::cout << "Magic cookie: 0x" << std::setw(8) << std::setfill('0')
            << header.magic_cookie << std::endl;
  std::cout << "Transaction ID: 0x" << std::setw(8) << std::setfill('0')
            << header.transaction_id[2] << std::setw(8) << std::setfill('0')
            << header.transaction_id[1] << std::setw(8) << std::setfill('0')
            << header.transaction_id[0] << std::endl;
  std::cout << "ATTRIBUTES\n";
  // ToDo
  std::cout << std::dec;
}

size_t StunHeader::serialize(std::span<uint8_t> target) const {
  if (target.size() < kStunHeaderSize)
    throw std::runtime_error("invalid message size");

  size_t offset = 0;

  uint16_t type = htons(encode_message_type(method, cclass));
  std::memcpy(target.data() + offset, &type, sizeof(type));
  offset += sizeof(type);

  uint16_t len = htons(message_length);
  std::memcpy(target.data() + offset, &len, sizeof(len));
  offset += sizeof(len);

  uint32_t cookie = htonl(magic_cookie);
  std::memcpy(target.data() + offset, &cookie, sizeof(cookie));
  offset += sizeof(cookie);

  std::memcpy(target.data() + offset, transaction_id, sizeof(transaction_id));
  offset += sizeof(transaction_id);

  return offset;
}

StunMessage StunMessage::deserialize(std::span<const uint8_t> src) {
  if (src.size() < sizeof(StunHeaderRaw)) {
    throw std::runtime_error(
        std::format("invalid message size: {}", src.size()));
  }
  StunMessage msg;

  StunHeaderRaw header_raw{};
  std::memcpy(reinterpret_cast<uint8_t *>(&header_raw), src.data(),
              sizeof(header_raw));

  auto type = ntohs(header_raw.type);
  auto magic_cookie = ntohl(header_raw.magic_cookie);
  auto message_length = ntohs(header_raw.message_length);

  auto [method, cclass] = decode_message_type(type);

  msg.header.method = method;
  msg.header.cclass = cclass;
  msg.header.message_length = message_length;
  msg.header.magic_cookie = magic_cookie;
  std::memcpy(msg.header.transaction_id, header_raw.transaction_id,
              sizeof(header_raw.transaction_id));

  const uint16_t expected_message_size =
      kStunHeaderSize + msg.header.message_length;

  if (src.size() != expected_message_size) {
    throw std::runtime_error(
        std::format("excpected message size is {}", expected_message_size,
                    ", but actual size is {}", src.size()));
  }

  return msg;
}

size_t StunMessage::serialize(std::span<uint8_t> target) const {
  size_t offset = 0;

  offset += header.serialize(target.subspan(offset));

  for (const auto &attr : attributes) {
    offset += attr.serialize(target.subspan(offset));
  }

  return offset;
}

StunMessage create_stun_request() {
  StunMessage message{};

  message.header.cclass = StunClass::request;
  message.header.method = StunMethod::binding;
  message.header.message_length = 0;
  message.header.magic_cookie = kMagicCookie;
  srand (static_cast<int>(time(NULL)));
  message.header.transaction_id[0] = std::rand();
  message.header.transaction_id[1] = std::rand();
  message.header.transaction_id[2] = std::rand();

  return message;
}

} // namespace stun
