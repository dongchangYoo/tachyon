#ifndef TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_H_
#define TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_H_

#include "tachyon/math/elliptic_curves/affine_point.h"
#include "tachyon/math/elliptic_curves/jacobian_point.h"
#include "tachyon/math/elliptic_curves/point_xyzz.h"
#include "tachyon/math/elliptic_curves/projective_point.h"
#include "tachyon/math/elliptic_curves/short_weierstrass/sw_curve_traits.h"

namespace tachyon::math {

// Config for Short Weierstrass model.
// See https://www.hyperelliptic.org/EFD/g1p/auto-shortw.html for more details.
// This config represents `y² = x³ + a * x + b`, where `a` and `b` are
// constants.
template <typename SWCurveConfig>
class SWCurve {
 public:
  using Config = SWCurveConfig;

  using BaseField = typename Config::BaseField;
  using ScalarField = typename Config::ScalarField;
  using AffinePointTy = typename SWCurveTraits<Config>::AffinePointTy;
  using ProjectivePointTy = typename SWCurveTraits<Config>::ProjectivePointTy;
  using JacobianPointTy = typename SWCurveTraits<Config>::JacobianPointTy;
  using PointXYZZTy = typename SWCurveTraits<Config>::PointXYZZTy;

  using CpuCurve = SWCurve<typename Config::CpuCurveConfig>;
  using GpuCurve = SWCurve<typename Config::GpuCurveConfig>;

  constexpr static bool kIsSWCurve = true;

  static void Init() {
    BaseField::Init();
    ScalarField::Init();

    Config::Init();
  }

  constexpr static bool IsOnCurve(const AffinePointTy& point) {
    if (point.infinity()) return false;
    BaseField right = point.x().Square() * point.x() + Config::kB;
    if constexpr (!Config::kAIsZero) {
      right += Config::kA * point.x();
    }
    return point.y().Square() == right;
  }

  constexpr static bool IsOnCurve(const ProjectivePointTy& point) {
    if (point.z().IsZero()) return false;
    BaseField z2 = point.z().Square();
    BaseField z3 = z2 * point.z();
    BaseField right = point.x().Square() * point.x() + Config::kB * z3;
    if constexpr (!Config::kAIsZero) {
      right += Config::kA * point.x() * z2;
    }
    return point.y().Square() * point.z() == right;
  }

  constexpr static bool IsOnCurve(const JacobianPointTy& point) {
    if (point.z().IsZero()) return false;
    BaseField z3 = point.z().Square() * point.z();
    BaseField right = point.x().Square() * point.x() + Config::kB * z3.Square();
    if constexpr (!Config::kAIsZero) {
      right += Config::kA * point.x() * z3 * point.z();
    }
    return point.y().Square() == right;
  }

  constexpr static bool IsOnCurve(const PointXYZZTy& point) {
    if (point.zzz().IsZero()) return false;
    BaseField right =
        point.x().Square() * point.x() + Config::kB * point.zzz().Square();
    if constexpr (!Config::kAIsZero) {
      right += Config::kA * point.x() * point.zz().Square();
    }
    return point.y().Square() == right;
  }
};

template <typename Config>
struct SWCurveTraits {
  using BaseField = typename Config::BaseField;
  using ScalarField = typename Config::ScalarField;
  using AffinePointTy = AffinePoint<SWCurve<Config>>;
  using ProjectivePointTy = ProjectivePoint<SWCurve<Config>>;
  using JacobianPointTy = JacobianPoint<SWCurve<Config>>;
  using PointXYZZTy = PointXYZZ<SWCurve<Config>>;
};

}  // namespace tachyon::math

#endif  // TACHYON_MATH_ELLIPTIC_CURVES_SHORT_WEIERSTRASS_SW_CURVE_H_
