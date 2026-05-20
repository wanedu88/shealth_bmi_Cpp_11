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

    for (int bandIndex = 0; bandIndex < SHealthConstants::kAgeBandCount; bandIndex++) {
        const int ageBandStart =
            SHealthConstants::kAgeBandStartMin + bandIndex * SHealthConstants::kAgeBandStep;
        printf(kAgeBandRatioFormat, ageBandStart, shealth.getBmiRatio(ageBandStart, underweightCode),
               shealth.getBmiRatio(ageBandStart, normalCode),
               shealth.getBmiRatio(ageBandStart, overweightCode),
               shealth.getBmiRatio(ageBandStart, obesityCode));
    }

    return 0;
}
