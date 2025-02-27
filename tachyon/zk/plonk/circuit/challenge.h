// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_CHALLENGE_H_
#define TACHYON_ZK_PLONK_CIRCUIT_CHALLENGE_H_

#include <string>

#include "absl/strings/substitute.h"

#include "tachyon/zk/plonk/circuit/phase.h"

namespace tachyon::zk {

class TACHYON_EXPORT Challenge {
 public:
  Challenge(size_t index, Phase phase) : index_(index), phase_(phase) {}

  size_t index() const { return index_; }
  Phase phase() const { return phase_; }

  std::string ToString() const {
    return absl::Substitute("{index: $0, phase: $1}", index_,
                            phase_.ToString());
  }

 private:
  size_t index_ = 0;
  Phase phase_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_CHALLENGE_H_
