// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CONSTANT_EXPRESSION_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CONSTANT_EXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/strings/substitute.h"

#include "tachyon/zk/plonk/circuit/expressions/expression.h"

namespace tachyon::zk {

template <typename F>
class ConstantExpression : public Expression<F> {
 public:
  static std::unique_ptr<ConstantExpression> CreateForTesting(const F& value) {
    return absl::WrapUnique(new ConstantExpression(value));
  }

  const F& value() const { return value_; }

  // Expression methods
  size_t Degree() const override { return 0; }

  uint64_t Complexity() const override { return 0; }

  std::string ToString() const override {
    return absl::Substitute("{type: $0, value: $1}",
                            ExpressionTypeToString(this->type_),
                            value_.ToString());
  }

 private:
  friend class ExpressionFactory<F>;

  explicit ConstantExpression(const F& value)
      : Expression<F>(ExpressionType::kConstant), value_(value) {}
  explicit ConstantExpression(F&& value)
      : Expression<F>(ExpressionType::kConstant), value_(std::move(value)) {}

  F value_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CONSTANT_EXPRESSION_H_
