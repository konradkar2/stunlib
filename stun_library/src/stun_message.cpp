#include "stun_library/stun_message.hpp"
#include "stun_library/stun_attribute_deserializer.hpp"
#include <iostream>
#include <format>

namespace stun {

struct StunHeaderRaw {
  uint16_t type;
  uint16_t message_length;
  uint32_t magic_cookie;
  uint32_t transaction_id[3];
};

static_assert(sizeof(StunHeaderRaw) == kStunHeaderSize);

namespace {

constexpr uint16_t kMethodBits0To3Mask = 0x000F;
constexpr uint16_t kMethodBits4To6Mask = 0x0070;
constexpr uint16_t kMethodBits7To11Mask = 0x0F80;
constexpr uint16_t kEncodedMethodBits4To6Mask = 0x00E0;
constexpr uint16_t kEncodedMethodBits7To11Mask = 0x3E00;
constexpr uint16_t kClassBit0Mask = 0x01;
constexpr uint16_t kClassBit1Mask = 0x02;
constexpr uint16_t kEncodedClassBit0Mask = 0x0010;
constexpr uint16_t kEncodedClassBit1Mask = 0x0100;
constexpr uint16_t kMethodBits4To6EncodeShift = 1;
constexpr uint16_t kMethodBits7To11EncodeShift = 2;
constexpr uint16_t kClassBit0EncodeShift = 4;
constexpr uint16_t kClassBit1EncodeShift = 7;
constexpr uint16_t kMethodBits4To6DecodeShift = 1;
constexpr uint16_t kMethodBits7To11DecodeShift = 2;
constexpr uint16_t kClassBit0DecodeShift = 4;
constexpr uint16_t kClassBit1DecodeShift = 7;
constexpr size_t kAttributeTypeFieldLength = sizeof(uint16_t);
constexpr size_t kAttributeLengthFieldLength = sizeof(uint16_t);
constexpr size_t kAttributeHeaderLength =
    kAttributeTypeFieldLength + kAttributeLengthFieldLength;
constexpr size_t kAttributePaddingAlignment = 4;

size_t padded_attribute_length(size_t length) {
  return (length + (kAttributePaddingAlignment - 1)) &
         ~(kAttributePaddingAlignment - 1);
}

} // namespace

uint16_t encode_message_type(StunMethod method, StunClass cls) {
  if (method != StunMethod::binding) {
    throw std::runtime_error("unimplemented method");
  }
  const uint16_t m = static_cast<uint16_t>(method);
  const uint16_t c = static_cast<uint16_t>(cls);
  uint16_t message_type =
      (m & kMethodBits0To3Mask) |
      ((m & kMethodBits4To6Mask) << kMethodBits4To6EncodeShift) |
      ((m & kMethodBits7To11Mask) << kMethodBits7To11EncodeShift) |
      ((c & kClassBit1Mask) << kClassBit1EncodeShift) |
      ((c & kClassBit0Mask) << kClassBit0EncodeShift);

  return message_type;
}

std::pair<StunMethod, StunClass> decode_message_type(uint16_t message_type) {
  uint16_t m =
      (message_type & kMethodBits0To3Mask) |
      ((message_type & kEncodedMethodBits4To6Mask) >>
       kMethodBits4To6DecodeShift) |
      ((message_type & kEncodedMethodBits7To11Mask) >>
       kMethodBits7To11DecodeShift);

  uint16_t c =
      ((message_type & kEncodedClassBit1Mask) >> kClassBit1DecodeShift) |
      ((message_type & kEncodedClassBit0Mask) >> kClassBit0DecodeShift);

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
  for (const auto& attr : attributes) {
    std::cout << "Type: 0x" << std::setw(2) << std::setfill('0')
            << static_cast<uint16_t>(attr->get_type()) << " (" << attr->get_type() << ")" << std::endl;
    std::cout << "Length 0x" << std::setw(2) << std::setfill('0') << attr->get_length() << std::endl;
    std::cout << "Value {\n";
    attr->print_value();
    std::cout << "}" << std::endl;
  }
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
  msg.attributes = StunAttributeDeserializer::deserialize(src.subspan(kStunHeaderSize), msg.header);
  return msg;
}

size_t StunMessage::serialize(std::span<uint8_t> target) const {
  size_t offset = 0;
  StunHeader serialized_header = header;
  serialized_header.message_length = 0;
  for (const auto &attr : attributes) {
    serialized_header.message_length += static_cast<uint16_t>(
        kAttributeHeaderLength + padded_attribute_length(attr->get_length()));
  }

  offset += serialized_header.serialize(target.subspan(offset));

  for (const auto &attr : attributes) {
    uint16_t type = htons(static_cast<uint16_t>(attr->get_type()));
    std::memcpy(target.data() + offset, &type, sizeof(type));
    offset += sizeof(type);

    uint16_t length = htons(attr->get_length());
    std::memcpy(target.data() + offset, &length, sizeof(length));
    offset += sizeof(length);

    offset += attr->serialize(target.subspan(offset));
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
