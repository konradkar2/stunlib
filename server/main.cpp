#include "server.hpp"
#include <iostream>

void print_usage(const char *program_name) {
  std::cout << "usage: " << program_name
            << " <port> [--xor-mapped-address|--mapped-address]\n";
}

bool parse_address_attribute_mode(int argc, char *argv[]) {
  if (argc == 2) {
    return true;
  }

  const std::string_view mode = argv[2];
  if (mode == "--xor-mapped-address") {
    return true;
  }

  if (mode == "--mapped-address") {
    return false;
  }

  throw std::runtime_error("unknown address attribute mode");
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    print_usage(argv[0]);
    return 1;
  }
  std::cout << "x";
  try {
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
    bool mode = parse_address_attribute_mode(argc, argv);
    stun::StunServer server(port, mode);
    server.run();

  } catch (const std::exception &ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
