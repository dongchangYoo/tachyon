#include "tachyon/zk/plonk/circuit/expressions/selector_expression.h"

#include "gtest/gtest.h"

#include "tachyon/math/elliptic_curves/bn/bn254/fr.h"

namespace tachyon::zk {

using Fr = math::bn254::Fr;

TEST(SelectorExpressionTest, Degree_Complexity) {
  std::unique_ptr<SelectorExpression<Fr>> expr =
      SelectorExpression<Fr>::CreateForTesting(Selector::Simple(1));
  EXPECT_EQ(expr->Degree(), size_t{1});
  EXPECT_EQ(expr->Complexity(), uint64_t{1});
}

}  // namespace tachyon::zk
