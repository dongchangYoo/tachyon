// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_SELECTOR_H_
#define TACHYON_ZK_PLONK_CIRCUIT_SELECTOR_H_

#include <stddef.h>

#include <string>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/strings/substitute.h"

#include "tachyon/export.h"

namespace tachyon::zk {

class TACHYON_EXPORT Selector {
 public:
  static Selector Simple(size_t index) { return {index, true}; }

  static Selector Complex(size_t index) { return {index, false}; }

  bool is_simple() const { return is_simple_; }

  std::string ToString() const {
    return absl::Substitute("{index: $0, is_simple: $1}", index_, is_simple_);
  }

 private:
  template <typename H>
  friend H AbslHashValue(H h, const Selector& selector);

  Selector(size_t index, bool is_simple)
      : index_(index), is_simple_(is_simple) {}

  size_t index_ = 0;
  bool is_simple_ = false;
};

template <typename H>
H AbslHashValue(H h, const Selector& selector) {
  return H::combine(std::move(h), selector.index_, selector.is_simple_);
}

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_SELECTOR_H_
