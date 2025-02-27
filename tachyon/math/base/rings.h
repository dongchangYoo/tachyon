#ifndef TACHYON_MATH_BASE_RINGS_H_
#define TACHYON_MATH_BASE_RINGS_H_

#include <type_traits>

#include "tachyon/base/template_util.h"
#include "tachyon/math/base/groups.h"

namespace tachyon::math {

// Ring is a set S with operations + and * that satisfies the followings:
// 1. Additive associativity: (a + b) + c = a + (b + c)
// 2. Additive commutativity: a + b = b + a
// 3. Additive identity: a + 0 = 0 + a = a
// 4. Additive inverse: a + (-a) = (-a) + a = 0
// 5. Distributivity: a * (b + c) = (a * b) + (a * c)
// 6. Multiplicative associativity: (a * b) * c = a * (b * c)
// See https://mathworld.wolfram.com/Ring.html

// The Ring supports SumOfProducts, inheriting the properties of both
// AdditiveGroup and MultiplicativeSemigroup.
template <typename F>
class Ring : public AdditiveGroup<F>, public MultiplicativeSemigroup<F> {
 public:
  // This is taken and modified from
  // https://github.com/arkworks-rs/algebra/blob/5dfeedf560da6937a5de0a2163b7958bd32cd551/ff/src/fields/mod.rs#L298C1-L305
  // Sum of products: a₁ * b₁ + a₂ * b₂ + ... + aₙ * bₙ
  template <
      typename InputIterator,
      std::enable_if_t<std::is_same_v<F, base::iter_value_t<InputIterator>>>* =
          nullptr>
  constexpr static F SumOfProducts(InputIterator a_first, InputIterator a_last,
                                   InputIterator b_first,
                                   InputIterator b_last) {
    F sum = F::Zero();
    while (a_first != a_last) {
      sum += (*a_first * *b_first);
      ++a_first;
      ++b_first;
    }
    return sum;
  }

  template <typename Container>
  constexpr static F SumOfProducts(const Container& a, const Container& b) {
    return SumOfProducts(std::begin(a), std::end(a), std::begin(b),
                         std::end(b));
  }
};

}  // namespace tachyon::math

#endif  // TACHYON_MATH_BASE_RINGS_H_
