#include "SHealth.h"
#include <cstdio>

namespace {
constexpr const char* kDefaultDataFile = "shealth.dat";
constexpr const char* kAgeBandRatioFormat =
    "%d - underweight = %f, normal = %f, overweight = %f, obesity = %f\n";
}  // namespace

int main() {
    SHealth shealth;
    shealth.calculateBmi(kDefaultDataFile);

    const int underweightCode = static_cast<int>(BmiCategoryCode::Underweight);
    const int normalCode = static_cast<int>(BmiCategoryCode::Normal);
    const int overweightCode = static_cast<int>(BmiCategoryCode::Overweight);
    const int obesityCode = static_cast<int>(BmiCategoryCode::Obesity);

    printf(kAgeBandRatioFormat, SHealthConstants::kAgeBandStartMin,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin, underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin, normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin, overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin, obesityCode));
    printf(kAgeBandRatioFormat, SHealthConstants::kAgeBandStartMin + SHealthConstants::kAgeBandStep,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + SHealthConstants::kAgeBandStep,
                               underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + SHealthConstants::kAgeBandStep,
                               normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + SHealthConstants::kAgeBandStep,
                               overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + SHealthConstants::kAgeBandStep,
                               obesityCode));
    printf(kAgeBandRatioFormat,
           SHealthConstants::kAgeBandStartMin + 2 * SHealthConstants::kAgeBandStep,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 2 * SHealthConstants::kAgeBandStep,
                               underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 2 * SHealthConstants::kAgeBandStep,
                               normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 2 * SHealthConstants::kAgeBandStep,
                               overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 2 * SHealthConstants::kAgeBandStep,
                               obesityCode));
    printf(kAgeBandRatioFormat,
           SHealthConstants::kAgeBandStartMin + 3 * SHealthConstants::kAgeBandStep,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 3 * SHealthConstants::kAgeBandStep,
                               underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 3 * SHealthConstants::kAgeBandStep,
                               normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 3 * SHealthConstants::kAgeBandStep,
                               overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 3 * SHealthConstants::kAgeBandStep,
                               obesityCode));
    printf(kAgeBandRatioFormat,
           SHealthConstants::kAgeBandStartMin + 4 * SHealthConstants::kAgeBandStep,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 4 * SHealthConstants::kAgeBandStep,
                               underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 4 * SHealthConstants::kAgeBandStep,
                               normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 4 * SHealthConstants::kAgeBandStep,
                               overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 4 * SHealthConstants::kAgeBandStep,
                               obesityCode));
    printf(kAgeBandRatioFormat,
           SHealthConstants::kAgeBandStartMin + 5 * SHealthConstants::kAgeBandStep,
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 5 * SHealthConstants::kAgeBandStep,
                               underweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 5 * SHealthConstants::kAgeBandStep,
                               normalCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 5 * SHealthConstants::kAgeBandStep,
                               overweightCode),
           shealth.getBmiRatio(SHealthConstants::kAgeBandStartMin + 5 * SHealthConstants::kAgeBandStep,
                               obesityCode));

    return 0;
}
