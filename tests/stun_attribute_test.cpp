#include "stun_library/stun_attribute.hpp"
#include "stun_library/error_code_attribute.hpp"
#include "stun_library/finger_print_attribute.hpp"
#include "stun_library/mapped_address_attribute.hpp"
#include "stun_library/xor_mapped_address_attribute.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace stun {
namespace {

TEST(StunAttribute, ReturnsTypeSetByConcreteAttributeConstructors) {
  const MappedAddressAttribute mapped_address;
  const XorMappedAddressAttribute xor_mapped_address;
  const ErrorCodeAttribute error_code;
  const FingerPrintAttribute finger_print;

  EXPECT_EQ(mapped_address.get_type(), AttributeTypeId::MappingAddress);
  EXPECT_EQ(xor_mapped_address.get_type(), AttributeTypeId::XorMappingAddress);
  EXPECT_EQ(error_code.get_type(), AttributeTypeId::ErrorCode);
  EXPECT_EQ(finger_print.get_type(), AttributeTypeId::FingerPrint);
}

TEST(StunAttribute, PrintsAttributeTypeNamesAndUnknownFallback) {
  std::ostringstream out;
  out << AttributeTypeId::MappingAddress << ' ' << AttributeTypeId::ErrorCode
      << ' ' << AttributeTypeId::XorMappingAddress << ' '
      << AttributeTypeId::FingerPrint << ' '
      << static_cast<AttributeTypeId>(0xFFFF);

  EXPECT_EQ(out.str(),
            "MappingAddress ErrorCode XorMappingAddress FingerPrint unknown");
}

} // namespace
} // namespace stun
