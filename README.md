# stunlib

[![Unit tests](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml)

C++20 STUN message library inspired by RFC 8489.

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

Headers live under the `stun_library/` prefix:

```cpp
#include "stun_library/stun_message.hpp"
```

## Basic message usage

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

If you want to do the socket part yourself, it looks roughly like this:

```cpp
#include "client.hpp"
#include "stun_library/stun_message.hpp"

#include <sys/socket.h>

int fd = socket(AF_INET, SOCK_DGRAM, 0);

stun::StunClient client("1.1.1.1", 3478);
client.send(fd, stun::create_stun_request());

stun::StunMessage response = client.receive(fd);
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

The repo also builds a very small IPv4 STUN server. It listens on UDP, accepts a
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

## Coverage

Coverage is calculated in GitHub Actions with `gcovr`. You can find the text
summary in the job output, and the full report is uploaded as an artifact.

To calculate it locally:

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DSTUN_ENABLE_COVERAGE=ON
cmake --build build-coverage --parallel
ctest --test-dir build-coverage --output-on-failure
gcovr --root . --filter 'stun_library/src/.*' --txt --print-summary
```
