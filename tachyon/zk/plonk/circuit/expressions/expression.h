// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_EXPRESSION_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_EXPRESSION_H_

#include <string>

#include "tachyon/base/logging.h"
#include "tachyon/zk/plonk/circuit/expressions/expression_type.h"

namespace tachyon::zk {

template <typename L, typename R>
struct Operands {
  L left;
  R right;
};

template <typename F>
class ExpressionFactory;

template <typename F, typename T>
class Evaluator;

template <typename F>
class ConstantExpression;

template <typename F>
class SelectorExpression;

template <typename F>
class FixedExpression;

template <typename F>
class AdviceExpression;

template <typename F>
class InstanceExpression;

template <typename F>
class ChallengeExpression;

template <typename F>
class NegatedExpression;

template <typename F>
class SumExpression;

template <typename F>
class ProductExpression;

template <typename F>
class ScaledExpression;

// A Expression represents a polynomial.
template <typename F>
class Expression {
 public:
  virtual ~Expression() = default;

  ExpressionType type() const { return type_; }

  // Returns the degree of the polynomial
  virtual size_t Degree() const = 0;

  // Returns the approximated computational complexity of this expression.
  virtual uint64_t Complexity() const = 0;
  virtual std::string ToString() const = 0;

  // Returns whether or not this expression contains a simple |Selector|.
  bool ContainsSimpleSelector() const;

  template <typename EvaluatedTy>
  EvaluatedTy Evaluate(const Evaluator<F, EvaluatedTy>* evaluator) const {
    return evaluator->Evaluate(this);
  }

#define DEFINE_CONVERSION_METHOD(type)                    \
  type##Expression<F>* To##type() {                       \
    CHECK_EQ(type_, ExpressionType::k##type);             \
    return static_cast<type##Expression<F>*>(this);       \
  }                                                       \
                                                          \
  const type##Expression<F>* To##type() const {           \
    CHECK_EQ(type_, ExpressionType::k##type);             \
    return static_cast<const type##Expression<F>*>(this); \
  }

  DEFINE_CONVERSION_METHOD(Constant)
  DEFINE_CONVERSION_METHOD(Selector)
  DEFINE_CONVERSION_METHOD(Fixed)
  DEFINE_CONVERSION_METHOD(Advice)
  DEFINE_CONVERSION_METHOD(Instance)
  DEFINE_CONVERSION_METHOD(Challenge)
  DEFINE_CONVERSION_METHOD(Negated)
  DEFINE_CONVERSION_METHOD(Sum)
  DEFINE_CONVERSION_METHOD(Product)
  DEFINE_CONVERSION_METHOD(Scaled)

#undef DEFINE_CONVERSION_METHOD

 protected:
  explicit Expression(ExpressionType type) : type_(type) {}

  ExpressionType type_;
};

template <typename F>
std::ostream& operator<<(std::ostream& os, const Expression<F>& expression) {
  return os << expression.ToString();
}

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_EXPRESSION_H_
