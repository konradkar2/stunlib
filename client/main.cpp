#include <cerrno>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>

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

std::string ipv4_to_string(uint32_t address) {
  in_addr addr{};
  addr.s_addr = htonl(address);

  char buffer[INET_ADDRSTRLEN]{};
  if (inet_ntop(AF_INET, &addr, buffer, sizeof(buffer)) == nullptr) {
    throw std::runtime_error("cannot format IPv4 address");
  }

  return buffer;
}

std::string ipv6_to_string(const std::array<uint32_t, 4> &address) {
  in6_addr addr{};
  for (size_t i = 0; i < address.size(); ++i) {
    const uint32_t word = htonl(address[i]);
    std::memcpy(addr.s6_addr + i * sizeof(word), &word, sizeof(word));
  }

  char buffer[INET6_ADDRSTRLEN]{};
  if (inet_ntop(AF_INET6, &addr, buffer, sizeof(buffer)) == nullptr) {
    throw std::runtime_error("cannot format IPv6 address");
  }

  return buffer;
}

std::optional<std::string> get_mapped_ip(const StunMessage &message) {
  for (const auto &attr : message.attributes) {
    if (attr->get_type() != AttributeTypeId::XorMappingAddress) {
      continue;
    }

    const auto &mapped = dynamic_cast<const XorMappedAddressAttribute &>(*attr);
    if (mapped.get_family() == AddressFamily::IPv4) {
      return ipv4_to_string(mapped.get_ipv4_address());
    }
    if (mapped.get_family() == AddressFamily::IPv6) {
      return ipv6_to_string(mapped.get_ipv6_address());
    }
  }

  for (const auto &attr : message.attributes) {
    if (attr->get_type() != AttributeTypeId::MappingAddress) {
      continue;
    }

    const auto &mapped = dynamic_cast<const MappedAddressAttribute &>(*attr);
    if (mapped.get_family() == AddressFamily::IPv4) {
      return ipv4_to_string(mapped.get_ipv4_address());
    }
    if (mapped.get_family() == AddressFamily::IPv6) {
      return ipv6_to_string(mapped.get_ipv6_address());
    }
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
 
  StunMessage response_message = client.receive(fd);
  response_message.print();

  if (const auto mapped_ip = get_mapped_ip(response_message)) {
    std::cout << "Mapped IP address: " << *mapped_ip << '\n';
  } else {
    std::cout << "No mapped IP address found in STUN response\n";
  }

}
