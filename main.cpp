#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <span>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "stun_client.hpp"

constexpr uint32_t kBufferSize = 1024 * 64;
constexpr uint32_t kMagicCookie = 0x2112A442;
constexpr uint32_t kStunHeaderSize = 20;

#define THROW_WITH_ERRNO(function)                                             \
  std::runtime_error(std::string{} + "failed to use " #function + ": " +       \
                     strerror(errno))

void dump_buffer(std::span<uint8_t> data) {
  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << static_cast<uint16_t>(data[i]);
  }
}

void udpSend(std::span<uint8_t> data, std::string address, uint16_t port) {

  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd < 0) {
    throw std::runtime_error("cannot open socket");
  }

  sockaddr_in servaddr{};
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;

  in_addr_t net_address = inet_addr(address.c_str());
  if (net_address == INADDR_NONE) {
    throw std::runtime_error("invalid ip address");
  }
  servaddr.sin_addr.s_addr = net_address;
  servaddr.sin_port = htons(port);

  ssize_t ret = sendto(fd, data.data(), data.size(), 0, (sockaddr *)&servaddr,
                       sizeof(servaddr));
  if (ret < 0) {
    THROW_WITH_ERRNO(sendto);
  }

  uint8_t buffor[kBufferSize];

  struct sockaddr response_addr{};
  socklen_t response_addr_len = sizeof(response_addr);
  int flags = 0;
  ssize_t received_n_bytes = recvfrom(fd, buffor, sizeof(buffor), flags,
                                      &response_addr, &response_addr_len);
  if (received_n_bytes < 0) {
    THROW_WITH_ERRNO(recvfrom);
  }

  dump_buffer(std::span{buffor, static_cast<size_t>(received_n_bytes)});
}

enum class StunMethod {
  binding = 1,
};

enum class StunClass {
  request = 0,
  indication = 1,
  success_response = 2,
  error_response = 3
};

struct StunMessageType {
    uint8_t padding : 2;
    uint8_t m11 : 1;
    uint8_t m10 : 1;
    uint8_t m9 : 1;
    uint8_t m8 : 1;
    uint8_t m7 : 1;
    uint8_t c1 : 1;
    uint8_t m6 : 1;
    uint8_t m5 : 1;
    uint8_t m4 : 1;
    uint8_t c0 : 1;
    uint8_t m3 : 1;
    uint8_t m2 : 1;
    uint8_t m1 : 1;
    uint8_t m0 : 1;
};
static_assert(sizeof(StunMessageType) == 2);

StunMessageType create_stun_message_type(StunMethod method,
                                         StunClass stun_class) {
  StunMessageType message_type{};

  if (method != StunMethod::binding) {
    throw std::runtime_error("unimplemented method");
  }
  message_type.m0 = 1; // binding

  switch (stun_class) {
  case StunClass::request:
    message_type.c1 = 0;
    message_type.c0 = 0;
    break;
  case StunClass::indication:
    message_type.c1 = 0;
    message_type.c0 = 1;
    break;
  case StunClass::success_response:
    message_type.c1 = 1;
    message_type.c0 = 0;
    break;
  case StunClass::error_response:
    message_type.c1 = 1;
    message_type.c0 = 1;
    break;
  default:
    throw std::runtime_error("invalid stun class provided");
  }

  return message_type;
}

uint16_t pack_stun_message_type(const StunMessageType &t) {
  uint16_t method =
      (t.m0  << 0)  | (t.m1  << 1)  | (t.m2  << 2)  | (t.m3  << 3) |
      (t.m4  << 4)  | (t.m5  << 5)  | (t.m6  << 6)  | (t.m7  << 7) |
      (t.m8  << 8)  | (t.m9  << 9)  | (t.m10 << 10) | (t.m11 << 11);
  uint16_t cls = (t.c1 << 1) | (t.c0 << 0);

  // Kodowanie zgodne z RFC5389 (wstawienie bitów klasy w odpowiednie pozycje)
  uint16_t type =
    (method & 0x000F) |              // M0-M3 -> bits 0..3
    ((method & 0x0070) << 1) |       // M4-M6 -> bits 5..7
    ((method & 0x0F80) << 2) |       // M7-M11 -> bits 9..13
    ((cls    & 0x02) << 7) |         // C1 -> bit 8
    ((cls    & 0x01) << 4);          // C0 -> bit 4

  return type;
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

  StunMessageType message_type = create_stun_message_type(StunMethod::binding, StunClass::request);
  header.message_type = htons(pack_stun_message_type(message_type));
  header.message_length = 0;
  header.magic_cookie = htonl(kMagicCookie);
  header.transaction_id[0] = 1;

  return header;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid arguments" << std::endl;
    return 1;
  }

  const std::string stun_address = argv[1];
  const uint16_t stun_port =
      static_cast<uint16_t>(std::stoi(std::string(argv[2])));

  stun_header header = create_stun_request();
  udpSend(std::span(reinterpret_cast<uint8_t *>(&header), sizeof(header)),
          stun_address, stun_port);

  std::cout << "goodbye!\n";
}