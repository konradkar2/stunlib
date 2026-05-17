#include <iostream>
#include <vector>
#include <span>
#include <iomanip>
#include <format>

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

struct StunHeader {
  uint16_t message_type;
  uint16_t message_length;
  uint32_t magic_cookie;
  uint32_t transaction_id[3];
};
static_assert(sizeof(StunHeader) == kStunHeaderSize);

// ToDo
struct StunAttribute {
  uint16_t type;
  uint16_t length;
  std::vector<uint8_t> value;
};

struct StunMessage
{
  StunHeader header;
  std::vector<StunAttribute> attributes;

  void print() {
    std::cout << "HEADER\n";
    std::cout << std::hex;
    std::cout << "Message type: 0x" << std::setw(4)  << std::setfill('0') << header.message_type << std::endl;
    std::cout << "Message length: 0x" << std::setw(4)  << std::setfill('0') << header.message_length << std::endl;
    std::cout << "Magic cookie: 0x" << std::setw(8)  << std::setfill('0') << header.magic_cookie << std::endl;
    std::cout << "Transaction ID: [2]: 0x" 
              << std::setw(8) << std::setfill('0') << header.transaction_id[2]
              << std::setw(8) << std::setfill('0') << header.transaction_id[1]
              << std::setw(8) << std::setfill('0') << header.transaction_id[0] << std::endl;
    std::cout << "ATTRIBUTES\n";
    //ToDo
    std::cout << std::dec;
  }
};

uint16_t pack_stun_message_type(StunMethod method,
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

StunMessage create_stun_request() {
  StunMessage message{};

  uint16_t message_type = pack_stun_message_type(StunMethod::binding, StunClass::request);
  message.header.message_type = htons(message_type);
  message.header.message_length = 0;
  message.header.magic_cookie = htonl(kMagicCookie);
  message.header.transaction_id[0] = 1;

  return message;
}

StunMessage convert_bytes_to_stun_message(std::span<uint8_t> data) {
  StunMessage message{};
  message.header.message_type = (static_cast<uint16_t>(data[0]) << 8) | 
                                (static_cast<uint16_t>(data[1]));
  message.header.message_length = (static_cast<uint16_t>(data[2]) << 8) | 
                                  (static_cast<uint16_t>(data[3]));
  const uint16_t expected_message_size = kStunHeaderSize + message.header.message_length; 
  if (data.size() != expected_message_size) {
    throw std::runtime_error(std::format("excpected message size is ", expected_message_size, ", but actual size is ", data.size()));
  }
  message.header.magic_cookie = (static_cast<uint32_t>(data[4]) << 24) | 
                                (static_cast<uint32_t>(data[5]) << 16) |
                                (static_cast<uint32_t>(data[6]) << 8) | 
                                (static_cast<uint32_t>(data[7]));
  message.header.transaction_id[2] = (static_cast<uint32_t>(data[8]) << 24) | 
                                     (static_cast<uint32_t>(data[9]) << 16) |
                                     (static_cast<uint32_t>(data[10]) << 8) | 
                                     (static_cast<uint32_t>(data[11]));
  message.header.transaction_id[1] = (static_cast<uint32_t>(data[12]) << 24) | 
                                     (static_cast<uint32_t>(data[13]) << 16) |
                                     (static_cast<uint32_t>(data[14]) << 8) | 
                                     (static_cast<uint32_t>(data[15]));
  message.header.transaction_id[0] = (static_cast<uint32_t>(data[16]) << 24) | 
                                     (static_cast<uint32_t>(data[17]) << 16) |
                                     (static_cast<uint32_t>(data[18]) << 8) | 
                                     (static_cast<uint32_t>(data[19]));
  return message;
}
