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

  void send(int fd, std::span<const uint8_t> data);
  void send(int fd, const StunMessage& message);
  StunMessage receive(int fd);
private:
  std::string m_server_ip;
  uint16_t m_server_port;
};

} // namespace stun
