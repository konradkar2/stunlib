#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "stun_message.hpp"
#include <arpa/inet.h>
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
#include <print>
#include "client.hpp"

using namespace stun;

void dump_buffer(std::span<uint8_t> data) {
  std::cout << "{";
  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << i << ":" << static_cast<uint16_t>(data[i]) << ", ";
  }
  std::cout << "}" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "invalid arguments: <adresss> <port>" << std::endl;
    return 1;
  }

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    throw std::runtime_error("cannot open socket");
  }

  const std::string stun_address = argv[1];
  const uint16_t stun_port =
      static_cast<uint16_t>(std::stoi(std::string(argv[2])));

  StunMessage request_message = create_stun_request();
  request_message.print();
  StunClient client(stun_address, stun_port);
  client.send(fd, request_message);
  printf("sizes of serialized stun message: %lu\n", sizeof(request_message));
 
 
  StunMessage response_message = client.receive(fd);
  response_message.print();

  printf("goodbye\n");
}
