#include <cerrno>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "stun_message/mapped_address_attribute.hpp"
#include "stun_message/stun_message.hpp"
#include "stun_message/xor_mapped_address_attribute.hpp"
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

std::string errno_message(const std::string &message, int error_number) {
  return message + ": " + std::strerror(error_number) + " (errno " +
         std::to_string(error_number) + ")";
}

struct MappedAddress {
  std::string ip;
  uint16_t port;
};

std::optional<MappedAddress> get_mapped_address(const StunMessage &message) {
  for (const auto &attr : message.attributes) {
    if (attr->get_type() != AttributeTypeId::XorMappingAddress) {
      continue;
    }

    const auto &mapped = dynamic_cast<const XorMappedAddressAttribute &>(*attr);
    return MappedAddress{mapped.get_ip_address().to_string(),
                         mapped.get_port()};
  }

  for (const auto &attr : message.attributes) {
    if (attr->get_type() != AttributeTypeId::MappingAddress) {
      continue;
    }

    const auto &mapped = dynamic_cast<const MappedAddressAttribute &>(*attr);
    return MappedAddress{mapped.get_ip_address().to_string(),
                         mapped.get_port()};
  }

  return std::nullopt;
}

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

  try {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
      throw std::runtime_error(errno_message("cannot open socket", errno));
    }

    const std::string stun_address = argv[1];
    const uint16_t stun_port =
        static_cast<uint16_t>(std::stoi(std::string(argv[2])));

    StunMessage request_message = create_stun_request();
    request_message.print();
    StunClient client(stun_address, stun_port);
    client.send(fd, request_message);

    StunMessage response_message = client.receive(fd);
    response_message.print();

    if (const auto mapped_address = get_mapped_address(response_message)) {
      std::cout << "Mapped IP address: " << mapped_address->ip << '\n';
      std::cout << "Mapped port: " << mapped_address->port << '\n';
    } else {
      std::cout << "No mapped address found in STUN response\n";
    }
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
