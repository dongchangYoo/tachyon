
#include "tachyon/zk/plonk/permutation/permutation_verifying_key.h"

#include "gtest/gtest.h"

#include "tachyon/base/buffer/vector_buffer.h"
#include "tachyon/math/elliptic_curves/bn/bn254/g1.h"

namespace tachyon::zk {

namespace {

class PermutationVerifyingKeyTest : public testing::Test {
 public:
  static void SetUpTestSuite() { math::bn254::G1Curve::Init(); }
};

}  // namespace

TEST_F(PermutationVerifyingKeyTest, Copyable) {
  using VerifyingKey = PermutationVerifyingKey<math::bn254::G1Curve>;

  VerifyingKey expected({Commitment<math::bn254::G1Curve>::Random()});
  VerifyingKey value;

  base::VectorBuffer write_buf;
  write_buf.Write(expected);

  write_buf.set_buffer_offset(0);

  write_buf.Read(&value);
  EXPECT_EQ(value, expected);
}

}  // namespace tachyon::zk
