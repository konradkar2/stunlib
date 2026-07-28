#include "stun_library/error_code_attribute.hpp"
#include "stun_library/mapped_address_attribute.hpp"
#include "stun_library/stun_message.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

enum class AddressAttributeMode {
  mapped_address,
  xor_mapped_address,
};

constexpr uint16_t kBadRequestErrorCode = 400;

std::string errno_message(const std::string &message, int error_number) {
  return message + ": " + std::strerror(error_number) + " (errno " +
         std::to_string(error_number) + ")";
}

void print_usage(const char *program_name) {
  std::cout << "usage: " << program_name
            << " <port> [--xor-mapped-address|--mapped-address]\n";
}

AddressAttributeMode parse_address_attribute_mode(int argc, char *argv[]) {
  if (argc == 2) {
    return AddressAttributeMode::xor_mapped_address;
  }

  const std::string_view mode = argv[2];
  if (mode == "--xor-mapped-address") {
    return AddressAttributeMode::xor_mapped_address;
  }

  if (mode == "--mapped-address") {
    return AddressAttributeMode::mapped_address;
  }

  throw std::runtime_error("unknown address attribute mode");
}

int create_server_socket(uint16_t port) {
  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    throw std::runtime_error(errno_message("cannot open UDP socket", errno));
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  if (bind(fd, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) <
      0) {
    const int error_number = errno;
    close(fd);
    throw std::runtime_error(
        errno_message("cannot bind UDP socket", error_number));
  }

  return fd;
}

stun::StunMessage create_binding_response(const stun::StunMessage &request,
                                          const sockaddr_in &client_address,
                                          AddressAttributeMode mode) {
  stun::StunMessage response{};
  response.header.method = stun::StunMethod::binding;
  response.header.cclass = stun::StunClass::success_response;
  response.header.message_length = 0;
  response.header.magic_cookie = stun::kMagicCookie;
  std::memcpy(response.header.transaction_id, request.header.transaction_id,
              sizeof(response.header.transaction_id));

  const uint16_t client_port = ntohs(client_address.sin_port);
  const uint32_t client_ip = ntohl(client_address.sin_addr.s_addr);

  if (mode == AddressAttributeMode::xor_mapped_address) {
    response.attributes.push_back(
        std::make_unique<stun::XorMappedAddressAttribute>(
            stun::XorMappedAddressAttribute::from_ip_address(
                client_port, stun::IpAddress::from_ipv4(client_ip),
                response.header.magic_cookie)));
  } else {
    response.attributes.push_back(std::make_unique<stun::MappedAddressAttribute>(
        stun::MappedAddressAttribute::from_ip_address(
            client_port, stun::IpAddress::from_ipv4(client_ip))));
  }

  return response;
}

stun::StunMessage create_error_response(const stun::StunMessage &request,
                                        uint16_t error_code,
                                        const std::string &reason_phrase) {
  stun::StunMessage response{};
  response.header.method = request.header.method;
  response.header.cclass = stun::StunClass::error_response;
  response.header.message_length = 0;
  response.header.magic_cookie = stun::kMagicCookie;
  std::memcpy(response.header.transaction_id, request.header.transaction_id,
              sizeof(response.header.transaction_id));

  response.attributes.push_back(std::make_unique<stun::ErrorCodeAttribute>(
      stun::ErrorCodeAttribute::create_error(error_code, reason_phrase)));

  return response;
}

void send_message(int fd, const stun::StunMessage &message,
                  const sockaddr_in &client_address,
                  socklen_t client_address_size) {
  std::array<uint8_t, stun::kBufferSize> response_buffer{};
  const size_t response_size = message.serialize(response_buffer);

  const ssize_t sent_size =
      sendto(fd, response_buffer.data(), response_size, 0,
             reinterpret_cast<const sockaddr *>(&client_address),
             client_address_size);
  if (sent_size < 0) {
    std::cerr << errno_message("failed to send response", errno) << '\n';
  }
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    print_usage(argv[0]);
    return 1;
  }

  try {
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
    const AddressAttributeMode mode = parse_address_attribute_mode(argc, argv);
    const int fd = create_server_socket(port);

    std::cout << "STUN server listening on 0.0.0.0:" << port << '\n';
    std::cout << "Address attribute mode: "
              << (mode == AddressAttributeMode::xor_mapped_address
                      ? "XOR-MAPPED-ADDRESS"
                      : "MAPPED-ADDRESS")
              << '\n';

    while (true) {
      std::array<uint8_t, stun::kBufferSize> request_buffer{};
      sockaddr_in client_address{};
      socklen_t client_address_size = sizeof(client_address);

      const ssize_t received_size =
          recvfrom(fd, request_buffer.data(), request_buffer.size(), 0,
                   reinterpret_cast<sockaddr *>(&client_address),
                   &client_address_size);
      if (received_size < 0) {
        std::cerr << errno_message("failed to receive packet", errno) << '\n';
        continue;
      }

      const auto received_packet = std::span<const uint8_t>{
          request_buffer.data(), static_cast<size_t>(received_size)};

      try {
        const stun::StunMessage request =
            stun::StunMessage::deserialize(received_packet);

        if (request.header.method != stun::StunMethod::binding) {
          std::cerr << "ignoring STUN message with unsupported method\n";
          continue;
        }

        if (request.header.cclass != stun::StunClass::request) {
          std::cerr << "sending 400 Bad Request for non-request STUN message\n";
          send_message(fd,
                       create_error_response(request, kBadRequestErrorCode,
                                             "Bad Request"),
                       client_address, client_address_size);
          continue;
        }

        send_message(fd, create_binding_response(request, client_address, mode),
                     client_address, client_address_size);
      } catch (const std::exception &ex) {
        std::cerr << "failed to handle STUN packet: " << ex.what() << '\n';
      }
    }
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
