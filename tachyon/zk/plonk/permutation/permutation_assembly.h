// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ASSEMBLY_H_
#define TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ASSEMBLY_H_

#include <utility>
#include <vector>

#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/openmp_util.h"
#include "tachyon/math/polynomials/univariate/univariate_evaluation_domain.h"
#include "tachyon/zk/plonk/permutation/cycle_store.h"
#include "tachyon/zk/plonk/permutation/lookup_table.h"
#include "tachyon/zk/plonk/permutation/permutation_argument.h"
#include "tachyon/zk/plonk/permutation/permutation_proving_key.h"
#include "tachyon/zk/plonk/permutation/permutation_verifying_key.h"

namespace tachyon::zk {

// Struct that accumulates all the necessary data in order to construct the
// permutation argument.
template <typename Curve, size_t MaxDegree>
class PermutationAssembly {
 public:
  constexpr size_t kRows = MaxDegree + 1;

  using ScalarField = typename Curve::ScalarField;
  using Evals = math::UnivariateEvaluations<ScalarField, MaxDegree>;
  using DensePoly = math::UnivariateDensePolynomial<ScalarField, MaxDegree>;

  // It has parameters for commitment and provide commit function.
  // TODO(dongchangYoo): substitute it to KZGParams, but not implemented yet.
  using ParamsTy = std::vector<math::AffinePoint<Curve>>;

  PermutationAssembly() = default;

  // Constructor with |PermutationArgument| which has copy permutation columns.
  explicit PermutationAssembly(const PermutationArgument& p)
      : PermutationAssembly(p.columns()) {}

  // Constructor with permutation columns.
  explicit PermutationAssembly(const std::vector<AnyColumn>& columns)
      : columns_(columns), cycle_store_(CycleStore(columns.size(), kRows)) {}

  const std::vector<AnyColumn>& columns() const { return columns_; }
  const CycleStore& cycle_store() const { return cycle_store_; }

  bool Copy(const AnyColumn& left_column, size_t left_row,
            const AnyColumn& right_column, size_t right_row) {
    CHECK_LE(left_row, kRows);
    CHECK_LE(right_row, kRows);

    // Get indices of each column.
    size_t left_col_idx = GetColumnIndex(left_column);
    size_t right_col_idx = GetColumnIndex(right_column);

    cycle_store_.MergeCycle(Label(left_col_idx, left_row),
                            Label(right_col_idx, right_row));
    return true;
  }

  // Returns |PermutationVerifyingKey| which has commitments for permutations.
  constexpr PermutationVerifyingKey<Curve> BuildVerifyingKey(
      math::UnivariateEvaluationDomain<ScalarField, MaxDegree>* domain) const {
    std::vector<Evals> permutations = GeneratePermutations(domain);

    // TODO(dongchangYoo): calculate commitments after complete Params. See
    // https://github.com/kroma-network/halo2/blob/7d0a36990452c8e7ebd600de258420781a9b7917/halo2_proofs/src/plonk/permutation/keygen.rs#L153-L162.
    Commitments<Curve> commitments;

    return PermutationVerifyingKey<Curve>(commitments);
  }

  // Returns the |PermutationProvingKey| that has the coefficient form and
  // evaluation form of the permutation.
  constexpr PermutationProvingKey<ScalarField, MaxDegree> BuildProvingKey(
      math::UnivariateEvaluationDomain<ScalarField, MaxDegree>* domain) const {
    // The polynomials of permutations in evaluation form.
    std::vector<Evals> permutations = GeneratePermutations(domain);

    // The polynomials of permutations with coefficients.
    std::vector<DensePoly> polys;
    polys.reserve(columns_.size());
    for (size_t i = 0; i < columns_.size(); ++i) {
      DensePoly poly = domain->IFFT(permutations[i]);
      polys.push_back(std::move(poly));
    }

    return PermutationProvingKey<ScalarField, MaxDegree>(
        std::move(permutations), std::move(polys));
  }

  // Generate the permutation polynomials based on the accumulated copy
  // permutations. Note that the permutation polynomials are in evaluation
  // form.
  std::vector<Evals> GeneratePermutations(
      math::UnivariateEvaluationDomain<ScalarField, MaxDegree>* domain) const {
    LookupTable<ScalarField, MaxDegree> lookup_table =
        LookupTable<ScalarField, MaxDegree>::Construct(columns_.size(), domain);

    // Init evaluation formed polynomials with all-zero coefficients
    std::vector<Evals> permutations =
        base::CreateVector(columns_.size(), Evals::Zero(MaxDegree));

    // Assign lookup_table to permutations
#if defined(TACHYON_HAS_OPENMP)
    size_t thread_num = omp_get_max_threads();
#else
    size_t thread_num = 1;
#endif  // defined(TACHYON_HAS_OPENMP)
    size_t chunk_size = (MaxDegree + thread_num) / thread_num;

    auto chunks = base::Chunked(permutations, chunk_size);
    std::vector<absl::Span<Evals>> chunks_vector =
        base::Map(chunks.begin(), chunks.end(),
                  [](const absl::Span<Evals>& chunk) { return chunk; });

    OPENMP_PARALLEL_FOR(size_t c = 0; c < chunks_vector.size(); ++c) {
      const absl::Span<Evals>& chunk = chunks_vector[c];

      size_t i = c * chunk_size;
      for (Evals& evals : chunk) {
        for (size_t j = 0; j <= MaxDegree; ++j) {
          *evals[j] = lookup_table[cycle_store_.GetNextLabel(Label(i, j))];
        }
        ++i;
      }
    }
    return permutations;
  }

 private:
  size_t GetColumnIndex(const AnyColumn& column) const {
    auto it = std::find(columns_.begin(), columns_.end(), column);
    CHECK_NE(it, columns_.end());
    return std::distance(columns_.begin(), it);
  }

  // Columns that participate on the copy permutation argument.
  std::vector<AnyColumn> columns_;
  CycleStore cycle_store_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_PERMUTATION_PERMUTATION_ASSEMBLY_H_
