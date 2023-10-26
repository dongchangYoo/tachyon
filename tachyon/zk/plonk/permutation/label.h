// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_PERMUTATION_LABEL_H_
#define TACHYON_ZK_PLONK_PERMUTATION_LABEL_H_

#include <stddef.h>

#include "tachyon/export.h"

namespace tachyon::zk {

struct TACHYON_EXPORT Label {
  size_t col = 0;
  size_t row = 0;

  Label(size_t col, size_t row) : col(col), row(row) {}

  bool operator==(const Label& other) const {
    return col == other.col && row == other.row;
  }
  bool operator!=(const Label& other) const { return !operator==(other); }
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PERMUTATION_LABEL_H_
