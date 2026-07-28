# stunlib

[![Unit tests](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/konradkar2/stunlib/actions/workflows/unit-tests.yml)

C++20 STUN message library inspired by RFC 8489.

`stunlib` is a small library for building and parsing STUN messages. The message
code is kept separate from the socket code, so you can use it with the included
example client or wire it into your own networking layer.

The usual flow is straightforward: create a binding request, serialize it, send
the bytes over UDP, then deserialize the response into a `StunMessage`.

## Using it from CMake

Add the project as a subdirectory and link your target with `stun_message`:

```cmake
add_subdirectory(stunlib)

target_link_libraries(your_target
    PRIVATE
        stun_message
)
```

Headers live under the `stun_message/` prefix:

```cpp
#include "stun_message/stun_message.hpp"
```

## Basic message usage

```cpp
#include "stun_message/stun_message.hpp"

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
and prints the decoded response:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/client/client 1.1.1.1 3478
```

If you want to do the socket part yourself, it looks roughly like this:

```cpp
#include "client.hpp"
#include "stun_message/stun_message.hpp"

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
#include "stun_message/xor_mapped_address_attribute.hpp"

for (const auto &attr : response.attributes) {
  if (attr->get_type() != stun::AttributeTypeId::XorMappingAddress) {
    continue;
  }

  const auto &mapped =
      dynamic_cast<const stun::XorMappedAddressAttribute &>(*attr);

  if (mapped.get_family() == stun::AddressFamily::IPv4) {
    uint32_t public_ip = mapped.get_ipv4_address();
  }
}
```

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
gcovr --root . --filter 'stun_message/src/.*' --txt --print-summary
```
