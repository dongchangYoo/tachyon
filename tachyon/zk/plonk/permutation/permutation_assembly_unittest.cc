#include "tachyon/zk/plonk/permutation/permutation_assembly.h"

#include <memory>

#include "gtest/gtest.h"

#include "tachyon/base/buffer/vector_buffer.h"
#include "tachyon/math/elliptic_curves/bn/bn254/g1.h"
#include "tachyon/math/polynomials/univariate/univariate_evaluation_domain_factory.h"

namespace tachyon::zk {
namespace {

constexpr static size_t MaxDegree = 7;

using Curve = math::bn254::G1Curve;
using F = typename Curve::ScalarField;
using Evals = typename PermutationAssembly<Curve, MaxDegree>::Evals;

class PermutationAssemblyTest : public testing::Test {
 public:
  static void SetUpTestSuite() { Curve::Init(); }

  void SetUp() override {
    expected_columns_ = {AnyColumn(0), AdviceColumn(1), FixedColumn(2),
                         InstanceColumn(3)};
    argment_ = PermutationArgument(expected_columns_);
    assembly_ = PermutationAssembly<Curve, MaxDegree>(argment_);

    domain_ = math::UnivariateEvaluationDomainFactory<F, MaxDegree>::Create(
        MaxDegree + 1);
  }

 protected:
  std::vector<AnyColumn> expected_columns_;
  PermutationArgument argment_;
  PermutationAssembly<Curve, MaxDegree> assembly_;
  std::unique_ptr<math::UnivariateEvaluationDomain<F, MaxDegree>> domain_;
};

}  // namespace

TEST_F(PermutationAssemblyTest, Constructor) {
  Mapping<Label> mapping = assembly_.mapping();
  Mapping<size_t> sizes = assembly_.sizes();

  for (size_t i = 0; i < expected_columns_.size(); ++i) {
    for (size_t j = 0; j <= MaxDegree; ++j) {
      EXPECT_EQ(mapping[Label(i, j)], Label(i, j));
      EXPECT_EQ(sizes[Label(i, j)], 1);
    }
  }

  EXPECT_EQ(assembly_.columns(), expected_columns_);
}

TEST_F(PermutationAssemblyTest, GeneratePermutation) {
  // Check initial permutation polynomials w/o any copy.
  std::vector<Evals> permutations =
      assembly_.GeneratePermutations(domain_.get());

  LookupTable<F, MaxDegree> lookup_table = LookupTable<F, MaxDegree>::Construct(
      expected_columns_.size(), domain_.get());

  for (size_t i = 0; i < assembly_.columns().size(); ++i) {
    std::vector<F> permutation = permutations[i].evaluations();
    for (size_t j = 0; j < permutation.size(); ++j) {
      EXPECT_EQ(permutation[j], lookup_table[Label(i, j)]);
    }
  }
}

TEST_F(PermutationAssemblyTest, Copy) {
  // Test example copy permutations: (1,3) -> (3, 5) -> (2,2)
  size_t cycle_len = 3;
  assembly_.Copy(expected_columns_[1], 3, expected_columns_[3], 5);
  assembly_.Copy(expected_columns_[3], 5, expected_columns_[2], 2);

  // Check mapping cycle.
  Mapping<Label> mapping = assembly_.mapping();
  Label base = Label(1, 3);
  Label cycle = base;
  for (size_t i = 0; i < cycle_len; ++i) {
    cycle = mapping[cycle];
  }
  EXPECT_EQ(cycle, base);

  // Members belonging to the same cycle have the same value in the aux
  Mapping<Label> aux = assembly_.aux_mapping();
  Label base_pos = Label(1, 3);
  EXPECT_EQ(aux[Label(1, 3)], base_pos);
  EXPECT_EQ(aux[Label(3, 5)], base_pos);
  EXPECT_EQ(aux[Label(2, 2)], base_pos);
}

TEST_F(PermutationAssemblyTest, KeysTest) {
  PermutationVerifyingKey<Curve> vk_ori =
      assembly_.BuildVerifyingKey(domain_.get());
  PermutationVerifyingKey<Curve> vk;

  PermutationProvingKey<F, MaxDegree> pk_ori =
      assembly_.BuildProvingKey(domain_.get());
  PermutationProvingKey<F, MaxDegree> pk;

  base::VectorBuffer buf;
  buf.WriteMany(vk_ori, pk_ori);

  buf.set_buffer_offset(0);
  buf.ReadMany(&vk, &pk);

  EXPECT_EQ(vk_ori.commitments(), vk.commitments());
  EXPECT_EQ(pk_ori.permutations(), pk.permutations());
  EXPECT_EQ(pk_ori.polys(), pk.polys());
}

}  // namespace tachyon::zk
