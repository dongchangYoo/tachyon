#include <iostream>

// clang-format off
#include "benchmark/poseidon/poseidon_config.h"
#include "benchmark/poseidon/poseidon_benchmark_runner.h"
#include "benchmark/poseidon/simple_poseidon_benchmark_reporter.h"
// clang-format on
#include "tachyon/base/logging.h"
#include "tachyon/c/math/elliptic_curves/bn/bn254/fr.h"
#include "tachyon/c/math/elliptic_curves/bn/bn254/g1.h"
#include "tachyon/math/elliptic_curves/bn/bn254/fr.h"

namespace tachyon {

using namespace crypto;

using Field = math::bn254::Fr;

extern "C" tachyon_bn254_fr* run_poseidon_arkworks(
    const tachyon_bn254_fr* pre_images, size_t aborbing_num,
    size_t squeezing_num, uint64_t* duration);

int RealMain(int argc, char** argv) {
  tachyon::PoseidonConfig config;
  if (!config.Parse(argc, argv)) {
    return 1;
  }

  Field::Init();
  SimplePoseidonBenchmarkReporter reporter("Poseidon Benchmark",
                                           config.repeating_num());
  reporter.AddVendor("arkworks");
  PoseidonBenchmarkRunner<Field> runner(&reporter, &config);

  std::vector<Field> results;
  results.reserve(config.repeating_num());
  runner.Run(&results);

  std::vector<Field> results_vendor;
  results_vendor.reserve(config.repeating_num());
  runner.RunExternal(run_poseidon_arkworks, &results_vendor);

  if (config.check_results()) {
    CHECK(results == results_vendor) << "Result not matched";
  }

  reporter.AddAverageToLastRow();
  reporter.Show();

  return 0;
}

}  // namespace tachyon

int main(int argc, char** argv) { return tachyon::RealMain(argc, argv); }
