// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CHALLENGE_EXPRESSION_H_
#define TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CHALLENGE_EXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/strings/substitute.h"

#include "tachyon/zk/plonk/circuit/challenge.h"
#include "tachyon/zk/plonk/circuit/expressions/expression.h"

namespace tachyon::zk {

template <typename F>
class ChallengeExpression : public Expression<F> {
 public:
  static std::unique_ptr<ChallengeExpression> CreateForTesting(
      const Challenge& challenge) {
    return absl::WrapUnique(new ChallengeExpression(challenge));
  }

  size_t index() const { return challenge_.index(); }
  Phase phase() const { return challenge_.phase(); }

  // Expression methods
  size_t Degree() const override { return 0; }

  uint64_t Complexity() const override { return 0; }

  std::string ToString() const override {
    return absl::Substitute("{type: $0, challenge: $1}",
                            ExpressionTypeToString(this->type_),
                            challenge_.ToString());
  }

 private:
  friend class ExpressionFactory<F>;

  explicit ChallengeExpression(const Challenge& challenge)
      : Expression<F>(ExpressionType::kChallenge), challenge_(challenge) {}

  Challenge challenge_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_EXPRESSIONS_CHALLENGE_EXPRESSION_H_
