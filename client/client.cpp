#include "client.hpp"

namespace stun {

void StunClient::send(int fd, std::span<const uint8_t> data) {
  sockaddr_in servaddr{};
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  
  in_addr_t net_address = inet_addr(m_server_ip.c_str());
  if (net_address == INADDR_NONE) {
    throw std::runtime_error("invalid ip address");
  }
  servaddr.sin_addr.s_addr = net_address;
  servaddr.sin_port = htons(m_server_port);

  ssize_t ret = sendto(fd, data.data(), data.size(), 0, (sockaddr *)&servaddr,
                       sizeof(servaddr));
  if (ret < 0) {
    throw std::runtime_error("error during sendind data");
  }
}

void StunClient::send(int fd, const StunMessage& message) {
  sockaddr_in servaddr{};
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  
  in_addr_t net_address = inet_addr(m_server_ip.c_str());
  if (net_address == INADDR_NONE) {
    throw std::runtime_error("invalid ip address");
  }
  servaddr.sin_addr.s_addr = net_address;
  servaddr.sin_port = htons(m_server_port);
  
  uint8_t data[kBufferSize];
  std::cout << "XD\n";
  size_t size = message.serialize(std::span{data, sizeof(data)});
  ssize_t ret = sendto(fd, data, size, 0, (sockaddr *)&servaddr,
                       sizeof(servaddr));
  if (ret < 0) {
    throw std::runtime_error("error during sendind data");
  }
}

StunMessage StunClient::receive(int fd) {
  uint8_t buffer[kBufferSize];

  struct sockaddr response_addr{};
  socklen_t response_addr_len = sizeof(response_addr);
  int flags = 0;
  ssize_t received_n_bytes = recvfrom(fd, buffer, sizeof(buffer), flags,
                                      &response_addr, &response_addr_len);

  if (received_n_bytes < 0) {
    throw std::runtime_error("error during receiving data");
  }

  StunMessage response = StunMessage::deserialize(
      std::span{buffer, static_cast<size_t>(received_n_bytes)});
  return response;
}

} //namespace stun
