#include "tachyon/crypto/hashes/prime_field_serializable.h"

#include "gtest/gtest.h"

#include "tachyon/math/finite_fields/test/gf7.h"

namespace tachyon::crypto {

namespace {

class PrimeFieldSerializableTest : public testing::Test {
 public:
  static void SetUpTestSuite() { math::GF7::Init(); }
};

}  // namespace

TEST_F(PrimeFieldSerializableTest, SerializeSingleValueToField) {
  constexpr int64_t kValue = 5;
  std::vector<math::GF7> fields;
  ASSERT_TRUE(SerializeToFieldElements(kValue, &fields));
  std::vector<math::GF7> expected = {math::GF7(5)};
  EXPECT_EQ(fields, expected);
}

TEST_F(PrimeFieldSerializableTest, SerializeBatchToField) {
  std::vector<int64_t> values = {1, 2, 3, 4, 5};
  std::vector<math::GF7> fields;
  ASSERT_TRUE(
      SerializeBatchToFieldElements(absl::MakeConstSpan(values), &fields));
  std::vector<math::GF7> expected = {
      math::GF7(1), math::GF7(2), math::GF7(3), math::GF7(4), math::GF7(5),
  };
  EXPECT_EQ(fields, expected);
}

TEST_F(PrimeFieldSerializableTest, SerializationFailureDueToModulus) {
  std::vector<math::GF7> fields;
  ASSERT_FALSE(
      SerializeToFieldElements(math::GF7::Config::kModulus[0], &fields));
  EXPECT_TRUE(fields.empty());
}

}  // namespace tachyon::crypto
