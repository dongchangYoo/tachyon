
#include "tachyon/zk/plonk/permutation/permutation_proving_key.h"

#include "gtest/gtest.h"

#include "tachyon/base/buffer/vector_buffer.h"
#include "tachyon/math/finite_fields/test/gf7.h"

namespace tachyon::zk {

namespace {

class PermutationProvingKeyTest : public testing::Test {
 public:
  static void SetUpTestSuite() { math::GF7::Init(); }
};

}  // namespace

TEST_F(PermutationProvingKeyTest, Copyable) {
  constexpr size_t kMaxDegree = 7;

  using ProvingKey = PermutationProvingKey<math::GF7, kMaxDegree>;
  using DensePoly = ProvingKey::DensePoly;
  using Evals = ProvingKey::Evals;

  ProvingKey expected({Evals::Random(kMaxDegree)},
                      {DensePoly::Random(kMaxDegree)});
  ProvingKey value;

  base::VectorBuffer write_buf;
  write_buf.Write(expected);

  write_buf.set_buffer_offset(0);

  write_buf.Read(&value);
  EXPECT_EQ(value, expected);
}

}  // namespace tachyon::zk
