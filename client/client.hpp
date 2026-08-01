#pragma once
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "stun_library/stun_message.hpp"
#include <span>

namespace stun {

class StunClient {
public:
  StunClient(std::string server_ip, uint16_t server_port) : m_server_ip(server_ip), m_server_port(server_port) {}
  virtual ~StunClient() {}

  void send(std::span<const uint8_t> data);
  void send(const StunMessage& message);
  StunMessage receive();
  void create_client_socket();
private:
  std::string m_server_ip;
  int m_socket_fd;
  uint16_t m_server_port;
};

} // namespace stun
