#include <string>
#include <cstdint>
#include "stun_library/stun_message.hpp"
#include "stun_library/error_code_attribute.hpp"
#include "stun_library/mapped_address_attribute.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"
#include <iostream>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace stun {
constexpr uint16_t kBadRequestErrorCode = 400;

class StunServer {
public:
  StunServer(uint16_t port, bool xor_mapped_mode);
  virtual ~StunServer();

  void run();
  
private:
  uint16_t m_port;
  int m_socket_fd;
  bool m_xor_mapped_mode;

  int create_server_socket();
  StunMessage create_binding_response(const StunMessage &request,
                                      const sockaddr_in &client_address);
  StunMessage create_error_response(const StunMessage &request,
                                    uint16_t error_code,
                                    const std::string &reason_phrase);
  void send_message(const stun::StunMessage &message,
                    const sockaddr_in &client_address,
                    socklen_t client_address_size);
  std::string errno_message(const std::string &message, int error_number);
};

} //namespace stun
