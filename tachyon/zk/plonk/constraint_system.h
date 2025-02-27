// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CONSTRAINT_SYSTEM_H_
#define TACHYON_ZK_PLONK_CONSTRAINT_SYSTEM_H_

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"

#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/containers/contains.h"
#include "tachyon/zk/plonk/circuit/constraint.h"
#include "tachyon/zk/plonk/circuit/expressions/evaluator/simple_selector_finder.h"
#include "tachyon/zk/plonk/circuit/gate.h"
#include "tachyon/zk/plonk/circuit/query.h"
#include "tachyon/zk/plonk/circuit/table_column.h"
#include "tachyon/zk/plonk/circuit/virtual_cells.h"
#include "tachyon/zk/plonk/lookup/lookup_argument.h"

namespace tachyon::zk {

// This is a description of the circuit environment, such as the gate, column
// and permutation arrangements.
template <typename F>
class ConstraintSystem {
 public:
  // NOTE(chokobole): This is different from the
  // |LookupArgument<F>::TableMapElem|.
  struct TableMapElem {
    std::unique_ptr<Expression<F>> input;
    TableColumn table;
  };

  using TableMap = std::vector<TableMapElem>;
  using AnnotateCallback = base::OnceCallback<std::string()>;
  using LookupCallback = base::OnceCallback<TableMap(VirtualCells&)>;
  using LookupAnyCallback =
      base::OnceCallback<LookupArgument::TableMap(VirtualCells&)>;
  using ConstrainCallback =
      base::OnceCallback<std::vector<Constraint>(VirtualCells&)>;

  size_t num_fixed_columns() const { return num_fixed_columns_; }

  size_t num_advice_columns() const { return num_advice_columns_; }

  size_t num_instance_columns() const { return num_instance_columns_; }

  size_t num_selectors() const { return num_selectors_; }

  size_t num_challenges() const { return num_challenges_; }

  const std::vector<Phase>& advice_column_phases() const {
    return advice_column_phases_;
  }

  const std::vector<Phase>& challenge_phases() const {
    return challenge_phases_;
  }

  const std::vector<FixedColumn>& selector_map() const { return selector_map_; }

  const std::vector<Gate<F>>& gates() const { return gates_; }

  const std::vector<AdviceQueryData>& advice_queries() const {
    return advice_queries_;
  }

  const std::vector<size_t>& num_advice_queries() const {
    return num_advice_queries_;
  }

  const std::vector<InstanceQueryData>& instance_queries() const {
    return instance_queries_;
  }

  const std::vector<FixedQueryData>& fixed_queries() const {
    return fixed_queries_;
  }

  const std::vector<LookupArgument>& lookups() const { return lookups_; }

  const absl::flat_hash_set<Column, std::string>& general_column_annotations()
      const {
    return general_column_annotations_;
  }

  const std::vector<FixedColumn>& constants() const { return constants_; }

  const std::optional<size_t>& minimum_degree() const {
    return minimum_degree_;
  }

  // Sets the minimum degree required by the circuit, which can be set to a
  // larger amount than actually needed. This can be used, for example, to
  // force the permutation argument to involve more columns in the same set.
  void set_minimum_degree(size_t degree) { minimum_degree_ = degree; }

  // Enables this fixed |column| to be used for global constant assignments.
  // The |column| will be equality-enabled, too.
  void EnableConstant(const FixedColumn& column) {
    // TODO(chokobole): should it be std::set<FixedColumn>?
    if (!base::Contains(constants_, column)) {
      constants_.push_back(column);
      EnableEquality(AnyColumn(column));
    }
  }

  // Enable the ability to enforce equality over cells in this column
  void EnableEquality(const AnyColumn& column) {
    // TODO(chokobole): should it be std::set<FixedColumn>?
    constants_.push_back(column);
    EnableEquality(column);
    // TODO(chokobole): enable this
    // permutation_.AddColumn(column);
  }

  // Add a lookup argument for some input expressions and table columns.
  //
  // |callback| returns a map between input expressions and the table columns
  // they need to match.
  size_t Lookup(const std::string_view& name, LookupCallback callback) {
    VirtualCells cells(this);
    LookupArgument<F>::TableMap table_map =
        base::Map(std::move(callback).Run(cells),
                  [&cells](const TableMapElement& element) {
                    CHECK(!element.input->ContainsSimpleSelector())
                        << "expression containing simple selector "
                           "supplied to lookup argument";

                    std::unique_ptr<Expression<F>> table = cells.QueryFixed(
                        element.table.column(), Rotation::cur());

                    return LookupArgument<F>::TableElement(std::move(input),
                                                           std::move(table));
                  });

    lookups_.push_back({name, std::move(table_map)});
    return lookups_.size() - 1;
  }

  // Add a lookup argument for some input expressions and table expressions.
  //
  // |table_map| returns a map between input expressions and the table
  // expressions they need to match.
  size_t LookupAny(const std::string_view& name, LookupAnyCallback callback) {
    VirtualCells cells(this);
    LookupArgument<F>::TableMap table_map = std::move(callback).Run(cells);

    lookups_.push_back({name, std::move(table_map)});
    return lookups_.size() - 1;
  }

  size_t QueryFixedIndex(const FixedColumn& column, Rotation at) {
    // Return existing query, if it exists
    size_t index;
    if (QueryIndex(fixed_queries_, column, at, &index)) return index;

    // Make a new query
    fixed_queries_.push_back({column, at});
    return fixed_queries_.size() - 1;
  }

  size_t QueryAdviceIndex(const AdviceColumn& column, Rotation at) {
    // Return existing query, if it exists
    size_t index;
    if (QueryIndex(advice_queries_, column, at, &index)) return index;

    // Make a new query
    advice_queries_.push_back({column, at});
    num_advice_queries_[column.index()] += 1;
    return advice_queries_.size() - 1;
  }

  size_t QueryInstanceIndex(const InstanceColumn& column, Rotation at) {
    // Return existing query, if it exists
    size_t index;
    if (QueryIndex(instance_queries_, column, at, &index)) return index;

    // Make a new query
    instance_queries_.push_back({column, at});
    return instance_queries_.size() - 1;
  }

  size_t QueryAnyIndex(const AnyColumn& column, Rotation at) {
    switch (column.type()) {
      case ColumnType::kFixed:
        return QueryFixedIndex(FixedColumn(column), at);
      case ColumnType::kAdvice:
        return QueryAdviceIndex(AdviceColumn(column), at);
      case ColumnType::kInstance:
        return QueryInstanceIndex(InstanceColumn(column), at);
      case ColumnType::kAny:
        break;
    }
    NOTREACHED();
    return 0;
  }

  size_t GetFixedQueryIndex(const FixedColumn& column, Rotation at) const {
    size_t index;
    CHECK(QueryIndex(fixed_queries_, column, at, &index));
    return index;
  }

  size_t GetAdviceQueryIndex(const AdviceColumn& column, Rotation at) const {
    size_t index;
    CHECK(QueryIndex(advice_queries_, column, at, &index));
    return index;
  }

  size_t GetInstanceQueryIndex(const InstanceColumn& column,
                               Rotation at) const {
    size_t index;
    CHECK(QueryIndex(instance_queries_, column, at, &index));
    return index;
  }

  size_t GetAnyQueryIndex(const AnyColumn& column, Rotation at) const {
    switch (column.type()) {
      case ColumnType::kFixed:
        return GetFixedQueryIndex(FixedColumn(column), at);
      case ColumnType::kAdvice:
        return GetAdviceQueryIndex(AdviceColumn(column), at);
      case ColumnType::kInstance:
        return GetInstanceQueryIndex(InstanceColumn(column), at);
      case ColumnType::kAny:
        break;
    }
    NOTREACHED();
    return 0;
  }

  // Creates a new gate.
  //
  // A gate is required to contain polynomial constraints. This method will
  // panic if |constrain| returns an empty iterator.
  void CreateGate(const std::string_view& name, ConstrainCallback constrain) {
    VirtualCells cells(this);
    std::vector<Constraint> constraints = std::move(constraints).Run(cells);
    std::vector<Selector> queried_selectors =
        std::move(cells).queried_selectors();
    std::vector<VirtualCell> queried_cells = std::move(cells).queried_cells();

    std::vector<std::string> constraint_names;
    std::vector<std::unique_ptr<Expression<F>>> polys;
    for (Constraint& constraint : constraints) {
      constraint_names.push_back(std::move(constraint).name());
      polys.push_back(std::move(constraint).expression());
    }
    CHECK(!polys.empty()) << "Gates must contain at least one constraint.";

    gates_.push_back({
        std::string(name),
        std::move(constraint_names),
        std::move(polys),
        std::move(queried_selectors),
        std::move(queried_cells),
    });
  }

  // Allocate a new simple selector. Simple selectors cannot be added to
  // expressions nor multiplied by other expressions containing simple
  // selectors. Also, simple selectors may not appear in lookup argument
  // inputs.
  Selector CreateSimpleSelector() { return Selector::Simple(num_selectors_++); }

  // Allocate a new complex selector that can appear anywhere
  // within expressions.
  Selector CreateComplexSelector() {
    return Selector::Complex(num_selectors_++);
  }

  // Allocate a new fixed column that can be used in a lookup table.
  TableColumn LookupTableColumn() { return TableColumn(CreateFixedColumn()); }

  // Annotate a Lookup column.
  void AnnotateLookupColumn(const TableColumn& column,
                            AnnotateCallback annotate) {
    // We don't care if the table has already an annotation. If it's the case we
    // keep the new one.
    general_column_annotations_.insert(
        ColumnData(ColumnType::kFixed, column.column().index()),
        std::move(annotate).Run());
  }

  // Annotate an Any column.
  void AnnotateLookupAnyColumn(const AnyColumn& column,
                               AnnotateCallback annotate) {
    // We don't care if the table has already an annotation. If it's the case we
    // keep the new one.
    general_column_annotations_.insert(
        ColumnData(column.type(), column.index()), std::move(annotate).Run());
  }

  // Allocate a new fixed column
  FixedColumn CreateFixedColumn() { return FixedColumn(num_fixed_columns_++); }

  // Allocate a new advice column at |kFirstPhase|.
  AdviceColumn CreateAdviceColumn(Phase phase) {
    return CreateAdviceColumn(kFirstPhase);
  }

  // Allocate a new advice column in given phase
  AdviceColumn CreateAdviceColumn(Phase phase) {
    Phase previous_phase;
    if (phase.Prev(&previous_phase)) {
      CHECK(base::Contains(advice_column_phases_, previous_phase))
          << "Phase " << previous_phase.ToString() << "is not used";
    }

    AdviceColumn column(num_advice_columns_++, phase);
    num_advice_queries_.push_back(0);
    advice_column_phases_.push_back(phase);
    return column;
  }

  // Allocate a new instance column
  InstanceColumn CreateInstanceColumn() {
    return InstanceColumn(num_instance_columns_++);
  }

  Challenge CreateChallengeUsableAfter(Phase phase) {
    CHECK(base::Contains(advice_column_phases_, phase))
        << "Phase " << phase.ToString() << "is not used";
    Challenge challenge(num_challenges_++, phase);
    challenge_phases_.push(phase);
    return challenge;
  }

  Phase ComputeMaxPhase() const {
    auto max_phase_it = std::max_element(advice_column_phases_.begin(),
                                         advice_column_phases_.end());
    if (max_phase_it == advice_column_phases_.end()) return 0;
    return *max_phase_it;
  }

  std::vector<Phase> GetPhases() const {
    Phase max_phase = ComputeMaxPhase();
    return base::CreateVector(size_t{max_phase}, [](size_t i) {
      return Phase(static_cast<uint8_t>(i));
    });
  }

  // Compute the degree of the constraint system (the maximum degree of all
  // constraints).
  size_t ComputeDegree() const {
    // The permutation argument will serve alongside the gates, so must be
    // accounted for.
    // TODO(chokobole): enable this.
    // size_t degree = permutation_.RequiredDegree();

    // The lookup argument also serves alongside the gates and must be accounted
    // for.
    size_t degree = std::max(degree, ComputeLookupRequiredDegree());

    // Account for each gate to ensure our quotient polynomial is the
    // correct degree and that our extended domain is the right size.
    degree = std::max(degree, ComputeGateRequiredDegree());

    return std::max(degree, minimum_degree_.value_or(1));
  }

  // Compute the number of blinding factors necessary to perfectly blind
  // each of the prover's witness polynomials.
  size_t ComputeBlindingFactors() const {
    // All of the prover's advice columns are evaluated at no more than
    auto max_num_advice_query_it = std::max_element(num_advice_queries_.begin(),
                                                    num_advice_queries_.end());
    size_t factors = max_num_advice_query_it == num_advice_queries_.end()
                         ? 1
                         : *max_num_advice_query_it;
    // distinct points during gate checks.

    // - The permutation argument witness polynomials are evaluated at most 3
    //   times.
    // - Each lookup argument has independent witness polynomials, and they are
    //   evaluated at most 2 times.
    size_t factors = std::max(3, factors);

    // Each polynomial is evaluated at most an additional time during
    // multiopen (at x₃ to produce qₑᵥₐₗₛ):
    ++factors;

    // h(x) is derived by the other evaluations so it does not reveal
    // anything; in fact it does not even appear in the proof.

    // h(x₃) is also not revealed; the verifier only learns a single
    // evaluation of a polynomial in x₁ which has h(x₃) and another random
    // polynomial evaluated at x₃ as coefficients -- this random polynomial
    // is "random_poly" in the vanishing argument.

    // Add an additional blinding factor as a slight defense against
    // off-by-one errors.
    return ++factors;
  }

  // Returns the minimum necessary rows that need to exist in order to
  // account for e.g. blinding factors.
  size_t ComputeMinimumRows() const {
    return ComputeBlindingFactors()  // m blinding factors
           + 1                       // for l_{-(m + 1)} (lₗₐₛₜ)
           + 1  // for l₀ (just for extra breathing room for the permutation
                // argument, to essentially force a separation in the
           // permutation polynomial between the roles of lₗₐₛₜ, l₀
           // and the interstitial values.)
           + 1  // for at least one row
  }

 private:
  template <typename QueryDataTy, typename ColumnTy>
  static bool QueryIndex(const std::vector<QueryDataTy>& queries,
                         const ColumnTy& column, Rotation at, size_t index) {
    // Return existing query, if it exists
    auto it = std::find_if(queries.begin(), queries.end(),
                           [&column, at](const QueryDataTy& query) {
                             return query.column() == column &&
                                    query.rotation() == at;
                           });
    if (it == queries.end()) return false;
    *index = std::distance(queries.begin(), it);
    return true;
  }

  size_t ComputeLookupRequiredDegree() const {
    std::vector<size_t> required_degrees =
        base::Map(lookups_, [](const LookupArgument& argument) {
          return argument.RequiredDegree();
        });
    auto max_required_degree =
        std::max_element(required_degrees.begin(), required_degrees.end());
    if (max_required_degree == required_degrees.end()) return 1;
    return *max_required_degree;
  }

  size_t ComputeGateRequiredDegree() const {
    std::vector<size_t> required_degrees =
        base::FlatMap(gates_, [](const Gate<F>& gate) {
          return base::Map(gate.polys,
                           [](const std::unique_ptr<Expression<F>>& poly) {
                             return poly->Degree()
                           });
        });
    auto max_required_degree =
        std::max_element(required_degrees.begin(), required_degrees.end());
    if (max_required_degree == required_degrees.end()) return 0;
    return *max_required_degree;
  }

  size_t num_fixed_columns_ = 0;
  size_t num_advice_columns_ = 0;
  size_t num_instance_columns_ = 0;
  size_t num_selectors_ = 0;
  size_t num_challenges_ = 0;

  // Contains the phase for each advice column. Should have same length as
  // num_advice_columns.
  std::vector<Phase> advice_column_phases_;
  // Contains the phase for each challenge. Should have same length as
  // num_challenges.
  std::vector<Phase> challenge_phases_;

  // This is a cached vector that maps virtual selectors to the concrete
  // fixed column that they were compressed into. This is just used by dev
  // tooling right now.
  std::vector<FixedColumn> selector_map_;
  std::vector<Gate<F>> gates_;
  std::vector<AdviceQueryData> advice_queries_;
  // Contains an integer for each advice column
  // identifying how many distinct queries it has
  // so far; should be same length as num_advice_columns.
  std::vector<size_t> num_advice_queries_;
  std::vector<InstanceQueryData> instance_queries_;
  std::vector<FixedQueryData> fixed_queries_;

  // Permutation argument for performing equality constraints.
  // TODO(chokobole): enable this.
  // std::vector<PermutationArgument> permutations_;

  // Vector of lookup arguments, where each corresponds
  // to a sequence of input expressions and a sequence
  // of table expressions involved in the lookup.
  std::vector<LookupArgument> lookups_;

  // List of indexes of Fixed columns which are associated to a
  // circuit-general Column tied to their annotation.
  absl::flat_hash_set<Column, std::string> general_column_annotations_;

  // Vector of fixed columns, which can be used to store constant values
  // that are copied into advice columns.
  std::vector<FixedColumn> constants_;

  std::optional<size_t> minimum_degree_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CONSTRAINT_SYSTEM_H_
