#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#include "SHealth.h"
#ifndef SHEALTH_TEST_FIXTURE_DIR
#define SHEALTH_TEST_FIXTURE_DIR "test/fixtures"
#endif

class SHealthBMITest : public ::testing::Test {
protected:
    void SetUp() override { health = SHealth(); }

    SHealth health;

    std::string fixturePath(const char* name) {
        return (std::filesystem::path(SHEALTH_TEST_FIXTURE_DIR) / name).string();
    }

    std::string writeTempCsv(const std::string& body) {
        const auto tempDir =
            std::filesystem::path(SHEALTH_TEST_FIXTURE_DIR) / "_tmp";
        std::filesystem::create_directories(tempDir);
        const auto path =
            tempDir / ("temp_" + std::to_string(tempCsvCounter_++) + ".csv");
        std::ofstream out(path);
        EXPECT_TRUE(out.is_open()) << path.string();
        out << body;
        return path.string();
    }

private:
    int tempCsvCounter_ = 0;
};

TEST_F(SHealthBMITest, Infrastructure_FixtureDirectory_Exists) {
    // Given: test_plan §2.1 test/fixtures/ 디렉터리
    // When:  fixturePath("tc01_bmi_normal.csv")의 부모 경로 확인
    // Then:  fixtures 디렉터리가 존재한다
    const std::filesystem::path fixturesDir(SHEALTH_TEST_FIXTURE_DIR);
    EXPECT_TRUE(std::filesystem::exists(fixturesDir))
        << "Missing fixtures dir: " << fixturesDir.string();
    EXPECT_TRUE(std::filesystem::is_directory(fixturesDir));
}

TEST_F(SHealthBMITest, Infrastructure_WriteTempCsv_CreatesReadableFile) {
    // Given: 최소 CSV 본문
    const std::string body = "id,age,weight,height\n1,25,70.0,170.0\n";
    // When:  writeTempCsv(body)
    const std::string path = writeTempCsv(body);
    // Then:  파일 존재·calculateBmi로 1건 로드 가능 (헬퍼·링크 스모크)
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(1, health.calculateBmi(path));
}

TEST_F(SHealthBMITest, TC_01_CalculatesBmi_NormalInput) {
    // Given: 25세, 70kg, 170cm, 20대 단일 레코드
    const std::string path = fixturePath("tc01_bmi_normal.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  recordCount==1; BMI≈24.22 → 과체중 100% (code 300)
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Overweight)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_02_CalculatesBmi_Height100cm) {
    // Given: 25세, 70kg, 100cm (height_m=1.0 → BMI=70.0)
    const std::string path = fixturePath("tc02_height_100.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  recordCount==1; 비만 100% (code 400)
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_03_CalculatesBmi_ReadmeSample) {
    // Given: README 샘플 79.5kg, 158.3cm, age=66 (60대)
    const std::string path = fixturePath("tc03_readme_sample.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  recordCount==1; BMI≈31.72 → 60대 비만 100% (code 400)
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(60, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_04_CalculatesBmi_LargeHeight) {
    // Given: 25세, 50kg, 250cm (BMI=8.0, 유한·>0)
    const std::string path = fixturePath("tc04_large_height.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  recordCount==1; 저체중 100%로 BMI>0·유한 간접 검증 (code 100)
    EXPECT_EQ(1, count);
    const double under =
        health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight));
    EXPECT_TRUE(std::isfinite(under));
    EXPECT_GT(under, 0.0);
    EXPECT_NEAR(100.0, under, 1e-2);
}

namespace {

double ratioSumForBand(SHealth& health, int ageClass) {
    return health.getBmiRatio(ageClass, static_cast<int>(BmiCategoryCode::Underweight)) +
           health.getBmiRatio(ageClass, static_cast<int>(BmiCategoryCode::Normal)) +
           health.getBmiRatio(ageClass, static_cast<int>(BmiCategoryCode::Overweight)) +
           health.getBmiRatio(ageClass, static_cast<int>(BmiCategoryCode::Obesity));
}

double distributionSum(const AgeBandDistribution& distribution) {
    return distribution.underweight + distribution.normal + distribution.overweight +
           distribution.obesity;
}

void expectSingleBandCategory(SHealth& health,
                              int count,
                              int ageClass,
                              BmiCategoryCode category) {
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(ageClass, static_cast<int>(category)),
                1e-2);
}

}  // namespace

TEST_F(SHealthBMITest, TC_06_ImputesWeight_BandAverage) {
    // Given: 20대 3명 weight 50, 60, 0 (동일 height 170)
    const std::string path = fixturePath("tc06_impute_three.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  0→55 보정; 3명 중 정상 2·저체중 1 → Normal≈66.67%, Under≈33.33%
    EXPECT_EQ(3, count);
    EXPECT_NEAR(66.666666,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
    EXPECT_NEAR(33.333333,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_07_AllWeightsZero_DivideByZero) {
    // Given: 20대 전원 weight=0 (nonZeroWeightCount=0)
    const std::string path = fixturePath("tc07_all_zero.csv");
    // When:  calculateBmi(path) — 현재: 0/0→NaN, classify None
    const int count = health.calculateBmi(path);
    // Then:  스냅샷 — 크래시 없음, 20대 4분류 비율 모두 0%
    EXPECT_EQ(2, count);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight)),
                1e-2);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Overweight)),
                1e-2);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
    EXPECT_NEAR(0.0, ratioSumForBand(health, 20), 1e-2);
}

TEST_F(SHealthBMITest, TC_08_IsolatesAgeBands) {
    // Given: 20대 50kg, 30대 0kg (30대 유효 표본 없음)
    const std::string path = fixturePath("tc08_band_isolation.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  20대만 저체중 100%; 30대는 20대 평균(50) 미적용 → 비율 0%
    EXPECT_EQ(2, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight)),
                1e-2);
    EXPECT_NEAR(0.0, ratioSumForBand(health, 30), 1e-2);
}

TEST_F(SHealthBMITest, TC_09_SingleValidSample) {
    // Given: 20대 유효 80kg 1명 + 0kg 1명
    const std::string path = fixturePath("tc09_single_valid.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  0→80 보정; 둘 다 BMI≈27.68 비만 → Obesity 100%
    EXPECT_EQ(2, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_10_BmiChangesAfterImpute) {
    // Given: TC_06과 동일 (0 포함 3명)
    const std::string path = fixturePath("tc06_impute_three.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  보정 후 분류 비율 합≈100% (0%만 나오지 않음)
    EXPECT_EQ(3, count);
    const double sum = ratioSumForBand(health, 20);
    EXPECT_GT(sum, 0.0);
    EXPECT_NEAR(100.0, sum, 1e-2);
}

TEST_F(SHealthBMITest, TC_11_Boundary_Underweight_18_5) {
    // Given: height=170, weight=53.464 → BMI≤18.5 (53.465는 float에서 >18.5)
    const std::string path = fixturePath("tc11_bmi_18_5.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  저체중 100% (code 100)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Underweight);
}

TEST_F(SHealthBMITest, TC_12_Boundary_Normal_Above18_5) {
    // Given: weight=53.47 → BMI>18.5
    const std::string path = fixturePath("tc12_bmi_normal_low.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  정상 100% (code 200)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Normal);
}

TEST_F(SHealthBMITest, TC_13_Boundary_Normal_Below23) {
    // Given: weight=66.467 → BMI≈22.999 (<23)
    const std::string path = fixturePath("tc13_bmi_normal_high.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  정상 100% (code 200)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Normal);
}

TEST_F(SHealthBMITest, TC_14_Boundary_Overweight_23) {
    // Given: weight=66.47 → BMI=23.0
    const std::string path = fixturePath("tc14_bmi_23.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  과체중 100% (code 300)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Overweight);
}

TEST_F(SHealthBMITest, TC_15_Boundary_Overweight_Below25) {
    // Given: weight=72.22 → BMI≈24.999
    const std::string path = fixturePath("tc15_bmi_24_99.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  과체중 100% (code 300)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Overweight);
}

TEST_F(SHealthBMITest, TC_16_Boundary_Obesity_25) {
    // Given: weight=72.25 → BMI=25.0 (README: ≥25 비만)
    const std::string path = fixturePath("tc16_bmi_25.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  비만 100% (code 400)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Obesity);
}

TEST_F(SHealthBMITest, TC_17_Boundary_Obesity_30) {
    // Given: weight=86.67 → BMI≈30.0
    const std::string path = fixturePath("tc17_bmi_30.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  비만 100% (code 400)
    expectSingleBandCategory(health, count, 20, BmiCategoryCode::Obesity);
}

TEST_F(SHealthBMITest, TC_18_Classification_ExclusiveComplete) {
    // Given: 20대 4명 — 저체중/정상/과체중/비만 경계 weight 각 1명
    const std::string path = fixturePath("tc18_four_categories.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  4분류 각 25%, 합≈100 (53.464/55/66.47/72.25; TC_16 Green 후 72.249→72.25 검토)
    EXPECT_EQ(4, count);
    EXPECT_NEAR(25.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight)),
                1e-2);
    EXPECT_NEAR(25.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
    EXPECT_NEAR(25.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Overweight)),
                1e-2);
    EXPECT_NEAR(25.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
    EXPECT_NEAR(100.0, ratioSumForBand(health, 20), 1e-2);
}

TEST_F(SHealthBMITest, TC_05_HeightZero_NoBandPeers_SkipsImpute) {
    // Given: 20대 1명 height=0, 동 연령대 유효 height 표본 없음
    const std::string path = fixturePath("tc05_height_zero.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  F-10 보정 생략(nonZeroHeightCount=0) → height 0 유지 → BMI inf → 비만 100%
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_34_ImputesHeight_BandAverage) {
    // Given: 20대 3명 height 170/0/180 — 0 행은 (170+180)/2=175.0 보정 (F-10)
    const std::string path = fixturePath("tc34_impute_heights.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  보정 후 3명 모두 BMI 정상 구간 → Normal 100%
    EXPECT_EQ(3, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_22_InvalidAgeClassAndType) {
    // Given: calculateBmi 완료 (tc01)
    ASSERT_EQ(1, health.calculateBmi(fixturePath("tc01_bmi_normal.csv")));
    // When:  잘못된 ageClass / type
    // Then:  각 0.0
    EXPECT_DOUBLE_EQ(0.0, health.getBmiRatio(19, 100));
    EXPECT_DOUBLE_EQ(0.0, health.getBmiRatio(80, 100));
    EXPECT_DOUBLE_EQ(0.0, health.getBmiRatio(20, 0));
    EXPECT_DOUBLE_EQ(0.0, health.getBmiRatio(20, 500));
}

TEST_F(SHealthBMITest, TC_23_GetBmiRatio_BeforeCalculate) {
    // Given: 기본 생성 SHealth (calculateBmi 미호출)
    // When:  getBmiRatio(20, 200)
    const double ratio = health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal));
    // Then:  0.0
    EXPECT_DOUBLE_EQ(0.0, ratio);
}

TEST_F(SHealthBMITest, TC_24_FileNotFound) {
    // Given: 존재하지 않는 경로
    const std::string path = fixturePath("no_such_file.dat");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  return 0
    EXPECT_EQ(0, count);
}

TEST_F(SHealthBMITest, TC_25_HeaderOnlyCsv) {
    // Given: 헤더만 있는 CSV
    const std::string path = fixturePath("tc25_header_only.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  return 0
    EXPECT_EQ(0, count);
}

TEST_F(SHealthBMITest, TC_26_ParseStops_OnBadLine) {
    // Given: 유효 1행 후 빈 줄(파싱 중단)
    const std::string path = fixturePath("tc26_bad_line.csv");
    // When:  calculateBmi(path)
    const int count = health.calculateBmi(path);
    // Then:  recordCount==1 (빈 줄에서 break, 이후 bad line 미처리)
    EXPECT_EQ(1, count);
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Overweight)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_31_Recalculate_OverwritesStats) {
    // Given: tc31_a(정상 100%), tc31_b(비만 100%)
    const std::string pathA = fixturePath("tc31_a.csv");
    const std::string pathB = fixturePath("tc31_b.csv");
    // When:  calculateBmi(a) → calculateBmi(b)
    ASSERT_EQ(1, health.calculateBmi(pathA));
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
    ASSERT_EQ(1, health.calculateBmi(pathB));
    // Then:  두 번째(b) 결과만 유효
    EXPECT_NEAR(100.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Obesity)),
                1e-2);
    EXPECT_NEAR(0.0,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-2);
}

TEST_F(SHealthBMITest, TC_33_GetAgeBandDistribution_ThreeCategories) {
    // Given: 20대 3명 — 저체중/정상/과체중 각 1명 (170cm)
    const std::string path = fixturePath("tc33_three_categories_20s.csv");
    // When:  calculateBmi(path) → getAgeBandDistribution(20)
    const int count = health.calculateBmi(path);
    const AgeBandDistribution dist = health.getAgeBandDistribution(20);
    // Then:  각 ~33.33%, 비만 0%, 합≈100; getBmiRatio와 일치
    EXPECT_EQ(3, count);
    EXPECT_NEAR(33.33, dist.underweight, 1e-2);
    EXPECT_NEAR(33.33, dist.normal, 1e-2);
    EXPECT_NEAR(33.33, dist.overweight, 1e-2);
    EXPECT_NEAR(0.0, dist.obesity, 1e-2);
    EXPECT_NEAR(100.0, distributionSum(dist), 1e-2);
    EXPECT_NEAR(dist.underweight,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Underweight)),
                1e-4);
    EXPECT_NEAR(dist.normal,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Normal)),
                1e-4);
    EXPECT_NEAR(dist.overweight,
                health.getBmiRatio(20, static_cast<int>(BmiCategoryCode::Overweight)),
                1e-4);
    // 잘못된 ageClass(25) → 전부 0
    const AgeBandDistribution invalid = health.getAgeBandDistribution(25);
    EXPECT_DOUBLE_EQ(0.0, invalid.underweight);
    EXPECT_DOUBLE_EQ(0.0, invalid.normal);
    EXPECT_DOUBLE_EQ(0.0, invalid.overweight);
    EXPECT_DOUBLE_EQ(0.0, invalid.obesity);
}
