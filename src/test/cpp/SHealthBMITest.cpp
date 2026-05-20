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
