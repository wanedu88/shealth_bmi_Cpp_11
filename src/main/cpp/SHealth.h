#pragma once

#include <istream>
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

constexpr int kCsvColId = 0;
constexpr int kCsvColAge = 1;
constexpr int kCsvColWeight = 2;
constexpr int kCsvColHeight = 3;
constexpr char kCsvDelimiter = ',';

constexpr double kHeightCmPerMeter = 100.0;
constexpr double kMissingWeight = 0.0;  // F-03: 체중 누락
constexpr double kMissingHeight = 0.0;  // F-10: 키 누락 (동일 0.0)
constexpr int kPercentMultiplier = 100;
}  // namespace SHealthConstants

// getBmiRatio(type) 인자 코드값 — public API 시그니처 유지
enum class BmiCategoryCode : int {
    Underweight = 100,
    Normal = 200,
    Overweight = 300,
    Obesity = 400
};

// F-09: 연령대별 4분류 BMI 분포 비율(%) — getBmiRatio와 동일 값
struct AgeBandDistribution {
    double underweight = 0.0;
    double normal = 0.0;
    double overweight = 0.0;
    double obesity = 0.0;
};

// F-12: 전체 로드 인원 대비 4분류 BMI 비율(%) — 연령대 무관
struct OverallBmiDistribution {
    double underweight = 0.0;
    double normal = 0.0;
    double overweight = 0.0;
    double obesity = 0.0;
};

class SHealth {
public:
    int calculateBmi(const std::string& filename);
    double getBmiRatio(int ageClass, int type);

    // ageClass ∈ {20, 30, …, 70} 만 유효; 그 외(19, 25, 80 등)는 전부 0.0 반환
    AgeBandDistribution getAgeBandDistribution(int ageClass) const;

    // F-11: classifyBmi Normal 슬롯(18.5 < BMI < 23) 사용자 ID 목록
    std::vector<int> getNormalBmiUserIds() const;

    // recordCount==0 이면 전부 0.0
    OverallBmiDistribution getOverallBmiDistribution() const;

private:
    enum class BmiClassSlot { None, Underweight, Normal, Overweight, Obesity };

    struct AgeBandRatios {
        double underweight = 0;
        double normal = 0;
        double overweight = 0;
        double obesity = 0;
    };

    int recordCount = 0;
    int ids[10000];
    int ages[10000];
    double heights[10000];
    double weights[10000];
    double bmis[10000];
    AgeBandRatios ageBandRatios[SHealthConstants::kAgeBandCount];
    AgeBandRatios overallRatios;

    // --- Parser: CSV load (DIP — stream vs file) ---
    bool loadRecordsFromFile(const std::string& filename);
    bool loadFromStream(std::istream& input);
    bool parseAndStoreLine(const std::string& line);
    std::vector<std::string> split(const std::string& line, char delimiter);

    // --- Domain + Statistics pipeline ---
    void runBmiPipeline();
    void imputeMissingWeightsByAgeBand();
    void imputeMissingHeightsByAgeBand();
    void computeAllBmis();
    BmiClassSlot classifyBmi(double bmi) const;
    void aggregateRatiosByAgeBand();
    void aggregateOverallRatios();

    void incrementClassificationCount(BmiClassSlot slot, int& underweightCount, int& normalCount,
                                      int& overweightCount, int& obesityCount) const;
    void fillRatiosFromCounts(AgeBandRatios& ratios, int underweightCount, int normalCount,
                              int overweightCount, int obesityCount, int memberCount) const;
    static AgeBandDistribution toDistribution(const AgeBandRatios& ratios);

    // --- Query helpers ---
    bool isInAgeBand(int age, int ageBandStart) const;
    int ageBandIndexFromStart(int ageBandStart) const;
    int ageBandIndexFromClass(int ageClass) const;
    double ratioForCategory(const AgeBandRatios& ratios, BmiCategoryCode category) const;
};
