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

    const std::vector<int> normalIds = shealth.getNormalBmiUserIds();
    printf("Normal BMI users (%zu):", normalIds.size());
    for (const int userId : normalIds) {
        printf(" %d", userId);
    }
    printf("\n");

    return 0;
}
