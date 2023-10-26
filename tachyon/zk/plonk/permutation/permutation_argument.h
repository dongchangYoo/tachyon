#ifndef TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ARGUMENT_H_
#define TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ARGUMENT_H_

#include <vector>

#include "tachyon/base/containers/contains.h"
#include "tachyon/zk/plonk/circuit/column.h"

namespace tachyon::zk {

// TODO(dongchangYoo): This class just provides shape of |PermutationArgument|.
class PermutationArgument {
 public:
  PermutationArgument() = default;
  explicit PermutationArgument(const std::vector<AnyColumn>& columns)
      : columns_(columns) {}

  void AddColumn(const AnyColumn& column) {
    if (base::Contains(columns_, column)) return;
    columns_.push_back(column);
  }

  const std::vector<AnyColumn>& columns() const { return columns_; }

 private:
  std::vector<AnyColumn> columns_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ARGUMENT_H_
