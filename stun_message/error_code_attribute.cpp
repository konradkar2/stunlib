#include "error_code_attribute.hpp"

namespace stun {

ErrorCodeAttribute ErrorCodeAttribute::create_error(uint16_t error_code, const std::string& phrase) {
  ErrorCodeAttribute ret {};
  ret.m_error_code = error_code;
  ret.m_reason_phrase = phrase;

  return ret;
}

AttributeTypeId ErrorCodeAttribute::get_type() const {
  return AttributeTypeId::ErrorCode;
}

uint16_t ErrorCodeAttribute::get_length() const {
  uint16_t length = 4 + m_reason_phrase.size();
  uint16_t length_padded = (length  + 3) & ~3;
  return length_padded;
}

void ErrorCodeAttribute::deserialize(std::span<const uint8_t> source) {
  uint8_t number;
  uint8_t classs;
  std::memcpy(&classs, source.data() + 2, 1);
  std::memcpy(&number, source.data() + 3, 1);
  m_error_code = 100 * classs + number;
  std::string reason(source.begin() + 4, source.end());
  m_reason_phrase = reason;
}

size_t ErrorCodeAttribute::serialize(std::span<uint8_t> target) const {
  size_t offset = 0;
  uint16_t padding = 0;
  std::memcpy(target.data() + offset, &padding, sizeof(padding));
  offset += sizeof(padding);

  uint8_t classs = m_error_code / 100;
  std::memcpy(target.data() + offset, &classs, sizeof(classs));
  offset += sizeof(classs);

  uint8_t number = m_error_code % 100;
  std::memcpy(target.data() + offset, &number, sizeof(number));
  offset += sizeof(number);

  std::memcpy(target.data() + offset, m_reason_phrase.data(), m_reason_phrase.size());
  offset += m_reason_phrase.size();

  return offset;
}

void ErrorCodeAttribute::print_value() const {
  std::cout << "Error code: " << m_error_code << std::endl;
  std::cout << "Reason phrase: " << m_reason_phrase << std::endl;
}

} // namespace stun
