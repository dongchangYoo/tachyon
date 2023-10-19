// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_COLUMN_TYPE_H_
#define TACHYON_ZK_PLONK_CIRCUIT_COLUMN_TYPE_H_

#include <ostream>
#include <string_view>

#include "tachyon/export.h"

namespace tachyon::zk {

// As the names denotes, |kFixed|, |kAdvice| and |kInstance| types
// represent fixed, advice and instance columns.
// |kAny| type of column can be either fixed, advice or instance column
enum class ColumnType {
  kAny,
  kFixed,
  kAdvice,
  kInstance,
};

TACHYON_EXPORT std::string_view ColumnTypeToString(ColumnType type);

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_COLUMN_TYPE_H_
