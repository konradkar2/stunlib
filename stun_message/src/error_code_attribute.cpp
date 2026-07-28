#include "stun_message/error_code_attribute.hpp"
#include <cstring>

namespace stun {
namespace {

constexpr size_t kReservedOffset = 0;
constexpr size_t kErrorClassOffset = 2;
constexpr size_t kErrorNumberOffset = 3;
constexpr size_t kReasonPhraseOffset = 4;
constexpr uint16_t kErrorClassMultiplier = 100;
constexpr size_t kAttributePaddingAlignment = 4;
constexpr uint16_t kReservedValue = 0;

size_t padded_attribute_length(size_t length) {
  return (length + (kAttributePaddingAlignment - 1)) &
         ~(kAttributePaddingAlignment - 1);
}

} // namespace

ErrorCodeAttribute ErrorCodeAttribute::create_error(uint16_t error_code, const std::string& phrase) {
  return ErrorCodeAttribute(error_code, phrase);
}

uint16_t ErrorCodeAttribute::get_length() const {
  return static_cast<uint16_t>(kReasonPhraseOffset + m_reason_phrase.size());
}

void ErrorCodeAttribute::deserialize(std::span<const uint8_t> source) {
  uint8_t number;
  uint8_t error_class;
  std::memcpy(&error_class, source.data() + kErrorClassOffset,
              sizeof(error_class));
  std::memcpy(&number, source.data() + kErrorNumberOffset, sizeof(number));
  m_error_code = kErrorClassMultiplier * error_class + number;
  std::string reason(source.begin() + kReasonPhraseOffset, source.end());
  m_reason_phrase = reason;
}

size_t ErrorCodeAttribute::serialize(std::span<uint8_t> target) const {
  uint16_t padding = kReservedValue;
  std::memcpy(target.data() + kReservedOffset, &padding, sizeof(padding));

  uint8_t error_class = m_error_code / kErrorClassMultiplier;
  std::memcpy(target.data() + kErrorClassOffset, &error_class,
              sizeof(error_class));

  uint8_t number = m_error_code % kErrorClassMultiplier;
  std::memcpy(target.data() + kErrorNumberOffset, &number, sizeof(number));

  std::memcpy(target.data() + kReasonPhraseOffset, m_reason_phrase.data(),
              m_reason_phrase.size());

  size_t raw_length = kReasonPhraseOffset + m_reason_phrase.size();
  size_t padded_length = padded_attribute_length(raw_length);
  size_t padding_needed = padded_length - raw_length;
  if (padding_needed) {
    std::memset(target.data() + raw_length, 0, padding_needed);
  }

  return padded_length;
}

void ErrorCodeAttribute::print_value() const {
  std::cout << "Error code: " << m_error_code << std::endl;
  std::cout << "Reason phrase: " << m_reason_phrase << std::endl;
}

} // namespace stun
