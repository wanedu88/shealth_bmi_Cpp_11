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

    for (int bandIndex = 0; bandIndex < SHealthConstants::kAgeBandCount; bandIndex++) {
        const int ageBandStart =
            SHealthConstants::kAgeBandStartMin + bandIndex * SHealthConstants::kAgeBandStep;
        const AgeBandDistribution dist = shealth.getAgeBandDistribution(ageBandStart);
        printf(kAgeBandRatioFormat, ageBandStart, dist.underweight, dist.normal, dist.overweight,
               dist.obesity);
    }

    return 0;
}
