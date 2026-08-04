#include "server.hpp"

namespace stun {

StunServer::StunServer(uint16_t port, bool xor_mapped_mode = true)
    : m_port(port), m_xor_mapped_mode(xor_mapped_mode) {}

StunServer::~StunServer() {
  close(m_socket_fd);
}

void StunServer::run() {
  create_server_socket();
  std::cout << "STUN server listening on 0.0.0.0:" << m_port << std::endl;
  std::cout << "Address attribute mode: "
            << (m_xor_mapped_mode ? "XOR-MAPPED-ADDRESS" : "MAPPED-ADDRESS")
            << std::endl;

  while (true) {
    std::array<uint8_t, stun::kBufferSize> request_buffer{};
    sockaddr_in client_address{};
    socklen_t client_address_size = sizeof(client_address);

    const ssize_t received_size = recvfrom(
        m_socket_fd, request_buffer.data(), request_buffer.size(), 0,
        reinterpret_cast<sockaddr *>(&client_address), &client_address_size);
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
        send_message(
            create_error_response(request, kBadRequestErrorCode, "Bad Request"),
            client_address, client_address_size);
        continue;
      }
      send_message(create_binding_response(request, client_address),
                   client_address, client_address_size);
    } catch (const std::exception &ex) {
      std::cerr << "failed to handle STUN packet: " << ex.what() << '\n';
    }
  }
}

void StunServer::create_server_socket() {
  m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (m_socket_fd < 0) {
    throw std::runtime_error(errno_message("cannot open UDP socket", errno));
  }
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(m_port);
  if (bind(m_socket_fd, reinterpret_cast<const sockaddr *>(&address),
           sizeof(address)) < 0) {
    const int error_number = errno;
    throw std::runtime_error(
        errno_message("cannot bind UDP socket", error_number));
  }
}

StunMessage
StunServer::create_binding_response(const StunMessage &request,
                                    const sockaddr_in &client_address) {
  StunMessage response{StunClass::success_response,
                       request.header.transaction_id};

  const uint16_t client_port = ntohs(client_address.sin_port);
  const uint32_t client_ip = ntohl(client_address.sin_addr.s_addr);

  auto ip = IpAddress::from_ipv4(client_ip);
  if (m_xor_mapped_mode) {
    response.attributes.push_back(std::make_unique<XorMappedAddressAttribute>(
        XorMappedAddressAttribute::from_ip_address(
            client_port, ip, response.header.magic_cookie)));
  } else {
    response.attributes.push_back(
        std::make_unique<stun::MappedAddressAttribute>(
            MappedAddressAttribute::from_ip_address(client_port, ip)));
  }
  return response;
}

StunMessage
StunServer::create_error_response(const StunMessage &request,
                                  uint16_t error_code,
                                  const std::string &reason_phrase) {
  StunMessage response{StunClass::error_response,
                       request.header.transaction_id};
  response.attributes.push_back(std::make_unique<stun::ErrorCodeAttribute>(
      ErrorCodeAttribute::create_error(error_code, reason_phrase)));

  return response;
}

void StunServer::send_message(const stun::StunMessage &message,
                              const sockaddr_in &client_address,
                              socklen_t client_address_size) {
  std::array<uint8_t, stun::kBufferSize> response_buffer{};
  const size_t response_size = message.serialize(response_buffer);

  const ssize_t sent_size = sendto(
      m_socket_fd, response_buffer.data(), response_size, 0,
      reinterpret_cast<const sockaddr *>(&client_address), client_address_size);
  if (sent_size < 0) {
    std::cerr << errno_message("failed to send response", errno) << '\n';
  }
}

std::string StunServer::errno_message(const std::string &message,
                                      int error_number) {
  return message + ": " + std::strerror(error_number) + " (errno " +
         std::to_string(error_number) + ")";
}

} // namespace stun
