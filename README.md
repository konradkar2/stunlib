# stunlib

[![Unit tests](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml)

C++20 STUN message library based on RFC 8489.

`stunlib` is a small library for building and parsing STUN messages. The message
code is kept separate from the socket code, so you can use it with the included
example client or wire it into your own networking layer.

The usual flow is straightforward: create a binding request, serialize it, send
the bytes over UDP, then deserialize the response into a `StunMessage`.

## Using it from CMake

Install the library, then find it from your project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix /path/to/install
```

Then link with the imported CMake target:

```cmake
find_package(stun REQUIRED)

target_link_libraries(your_target
    PRIVATE
        stun::stun_library
)
```

Headers can be found under `stun_library/` prefix:

```cpp
#include "stun_library/stun_message.hpp"
```

## Basic usage

```cpp
#include "stun_library/stun_message.hpp"

#include <array>
#include <cstdint>
#include <span>

int main() {
  stun::StunMessage request = stun::create_stun_request();

  std::array<uint8_t, stun::kBufferSize> buffer{};
  const size_t size = request.serialize(buffer);

  // send buffer[0..size] through your UDP socket

  std::array<uint8_t, stun::kBufferSize> response_buffer{};
  size_t response_size = 0;
  // receive bytes from your UDP socket into response_buffer
  // and set response_size to the number of received bytes

  stun::StunMessage decoded =
      stun::StunMessage::deserialize(
          std::span<const uint8_t>{response_buffer.data(), response_size});
  decoded.print();
}
```

## Example UDP client

There is also a tiny client target in this repo. It sends a STUN binding request
and prints the decoded response, including the mapped IP address and port:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/client/client 74.125.250.129 19302
```

If you want to do the socket part yourself:

```cpp
#include "client.hpp"
#include "stun_library/stun_message.hpp"

#include <sys/socket.h>

stun::StunClient client("1.1.1.1", 3478);
client.create_client_socket();
client.send(stun::create_stun_request());

stun::StunMessage response = client.receive();
response.print();
```

To pull the mapped IP address out of the response, look for
`XOR-MAPPED-ADDRESS` first. Most STUN servers return that one:

```cpp
#include "stun_library/ip_address.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"

for (const auto &attr : response.attributes) {
  if (attr->get_type() != stun::AttributeTypeId::XorMappingAddress) {
    continue;
  }

  const auto &mapped =
      dynamic_cast<const stun::XorMappedAddressAttribute &>(*attr);

  stun::IpAddress public_ip = mapped.get_ip_address();
  uint16_t public_port = mapped.get_port();
}
```

## Example UDP server

The repo also builds a very small IPv4 STUN UDP server. It acceps
Binding Request, and sends back a Binding Success Response filled from the
client's source IP and port. By default it uses `XOR-MAPPED-ADDRESS`, which is
what modern STUN clients usually expect. You can switch it to the older
`MAPPED-ADDRESS` attribute if you want to test that path.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/server/server 3478 --xor-mapped-address
./build/server/server 3478 --mapped-address
```

You can try it from another terminal with the example client:

```bash
./build/client/client 127.0.0.1 3478
```

If you build a response yourself, mapped attributes are created from an
`IpAddress`:

```cpp
#include "stun_library/ip_address.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"

auto attr = stun::XorMappedAddressAttribute::from_ip_address(
    client_port,
    stun::IpAddress::from_ipv4(client_ip),
    stun::kMagicCookie
);
```

If the server receives a parsed STUN Binding message that is not a request, it
replies with an Error Response and an `ERROR-CODE` attribute.

## Unit tests

Unit tests run in GitHub Actions on every push and pull request.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## End-to-end tests

This repository includes two simple end-to-end shell tests that exercise the
example `client` and `server` binaries. You can find them under
`tests/e2e_tests/`.

- [tests/e2e_tests/client_to_google_server_test.sh](tests/e2e_tests/client_to_google_server_test.sh) — runs the example `client` against
  Google's public STUN server at `74.125.250.129:19302` and writes output to
  `tests/e2e_tests/client_to_google_server.log`. It greps the log for the
  `XOR-MAPPED-ADDRESS` (or `MAPPED-ADDRESS`) attribute and prints a colored
  `TEST PASSED` / `TEST FAILED` message.

- [tests/e2e_tests/client_to_server_test.sh](tests/e2e_tests/client_to_server_test.sh) — starts the local example `server`,
  runs the example `client` against it, writes separate `server.log` and
  `client.log` files under `tests/e2e_tests/`, and reports pass/fail based on
  the client's output.

Both scripts expect the example binaries to be built in `build/`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Make the scripts executable and run them:

```bash
chmod +x tests/e2e_tests/*.sh
./tests/e2e_tests/client_to_google_server_test.sh
./tests/e2e_tests/client_to_server_test.sh
```

If a test fails, inspect the corresponding log(s) in `tests/e2e_tests/`.
```
