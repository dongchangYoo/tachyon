// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_REGION_COLUMN_H_
#define TACHYON_ZK_PLONK_CIRCUIT_REGION_COLUMN_H_

#include <utility>

#include "absl/hash/hash.h"

#include "tachyon/export.h"
#include "tachyon/zk/plonk/circuit/column.h"
#include "tachyon/zk/plonk/circuit/selector.h"

namespace tachyon::zk {

class TACHYON_EXPORT RegionColumn {
  enum class Type {
    kColumn,
    kSelector,
  };

  RegionColumn() = default;
  explicit RegionColumn(const AnyColumn& column)
      : type_(Type::kColumn), column_(column) {}
  explicit RegionColumn(const Selector& selector)
      : type_(Type::kSelector), selector_(selector) {}

  Type type() const { return type_; }

 private:
  Type type_;
  union {
    AnyColumn column_;
    Selector selector_;
  };
};

template <typename H>
H AbslHashValue(H h, const RegionColumn& m) {
  if (m.type_ == RegionColumn::Type::kColumn) {
    return H::combine(std::move(h), m.column_);
  } else {
    return H::combine(std::move(h), m.selector_);
  }
}

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_REGION_COLUMN_H_
