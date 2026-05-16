#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <netinet/in.h>
#include <span>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <span>
#include "stun_message.hpp"


#define THROW_WITH_ERRNO(function)                                             \
  std::runtime_error(std::string{} + "failed to use " #function + ": " +       \
                     strerror(errno))

void dump_buffer(std::span<uint8_t> data) {
  std::cout << "{";
  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << i << ":" << static_cast<uint16_t>(data[i]) << ", ";
  }
  std::cout << "}" << std::endl;
}

void udp_send(std::span<uint8_t> data, std::string address, uint16_t port) {

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

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid arguments" << std::endl;
    return 1;
  }

  const std::string stun_address = argv[1];
  const uint16_t stun_port =
      static_cast<uint16_t>(std::stoi(std::string(argv[2])));

  StunMessage message = create_stun_request();
  message.print_info();
  udp_send(std::span(reinterpret_cast<uint8_t *>(&message.header), sizeof(message.header)),
          stun_address, stun_port);

  std::cout << "goodbye!\n";
}