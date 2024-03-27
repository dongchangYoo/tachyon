#include "benchmark/msm/simple_msm_benchmark_reporter.h"

#include <string>
#include <vector>

#include "tachyon/base/containers/container_util.h"
#include "tachyon/base/strings/string_number_conversions.h"

namespace tachyon {

SimpleMSMBenchmarkReporter::SimpleMSMBenchmarkReporter(
    std::string_view title, const std::vector<uint64_t>& exponents)
    : SimpleBenchmarkReporter(title), exponents_(exponents) {
  targets_ = base::Map(exponents, [](uint64_t exponent) {
    return base::NumberToString(exponent);
  });
  times_.resize(exponents.size());
  AddVendor("tachyon");
}

void SimpleMSMBenchmarkReporter::AddVendor(std::string_view name) {
  column_headers_.push_back(std::string(name));
}

}  // namespace tachyon
