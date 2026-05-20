#pragma once

#include <string>
#include <vector>

// README F-05 BMI 분류 경계 및 도메인 상수
namespace SHealthConstants {
constexpr double kBmiUnderMax = 18.5;        // 저체중: BMI ≤ 18.5
constexpr double kBmiNormalMax = 23.0;         // 정상 상한(미만): 18.5 초과 ~ 23 미만
constexpr double kBmiOverweightMax = 25.0;   // 과체중 상한(미만): 23 이상 ~ 25 미만

constexpr int kAgeBandStartMin = 20;
constexpr int kAgeBandStartMax = 70;
constexpr int kAgeBandStep = 10;
constexpr int kAgeBandWidth = 10;
constexpr int kAgeBandCount = (kAgeBandStartMax - kAgeBandStartMin) / kAgeBandStep + 1;

constexpr int kCsvColAge = 1;
constexpr int kCsvColWeight = 2;
constexpr int kCsvColHeight = 3;
constexpr char kCsvDelimiter = ',';

constexpr double kHeightCmPerMeter = 100.0;
constexpr double kMissingWeight = 0.0;
constexpr int kPercentMultiplier = 100;
}  // namespace SHealthConstants

// getBmiRatio(type) 인자 코드값 — public API 시그니처 유지
enum class BmiCategoryCode : int {
    Underweight = 100,
    Normal = 200,
    Overweight = 300,
    Obesity = 400
};

class SHealth {
public:
    int calculateBmi(const std::string& filename);
    double getBmiRatio(int ageClass, int type);

private:
    enum class BmiClassSlot { None, Underweight, Normal, Overweight, Obesity };

    struct AgeBandRatios {
        double underweight = 0;
        double normal = 0;
        double overweight = 0;
        double obesity = 0;
    };

    int recordCount = 0;
    int ages[10000];
    double heights[10000];
    double weights[10000];
    double bmis[10000];
    AgeBandRatios ageBandRatios[SHealthConstants::kAgeBandCount];

    bool loadRecordsFromFile(const std::string& filename);
    bool parseAndStoreLine(const std::string& line);
    void imputeMissingWeightsByAgeBand();
    void computeAllBmis();
    BmiClassSlot classifyBmi(double bmi) const;
    void aggregateRatiosByAgeBand();

    bool isInAgeBand(int age, int ageBandStart) const;
    int ageBandIndexFromStart(int ageBandStart) const;
    int ageBandIndexFromClass(int ageClass) const;
    double ratioForCategory(const AgeBandRatios& ratios, BmiCategoryCode category) const;

    std::vector<std::string> split(const std::string& line, char delimiter);
};
