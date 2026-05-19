#pragma once

#include <string>
#include <vector>

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
    int recordCount = 0;
    int ages[10000];
    double heights[10000];
    double weights[10000];
    double bmis[10000];

    // 저체중 비율(%) — suffix: ageBand 시작 연령(20, 30, …, 70)
    double underweight20 = 0, underweight30 = 0, underweight40 = 0;
    double underweight50 = 0, underweight60 = 0, underweight70 = 0;
    // 정상체중 비율(%) — suffix: ageBand 시작 연령
    double normalweight20 = 0, normalweight30 = 0, normalweight40 = 0;
    double normalweight50 = 0, normalweight60 = 0, normalweight70 = 0;
    // 과체중 비율(%) — suffix: ageBand 시작 연령
    double overweight20 = 0, overweight30 = 0, overweight40 = 0;
    double overweight50 = 0, overweight60 = 0, overweight70 = 0;
    // 비만 비율(%) — suffix: ageBand 시작 연령
    double obesity20 = 0, obesity30 = 0, obesity40 = 0;
    double obesity50 = 0, obesity60 = 0, obesity70 = 0;

    std::vector<std::string> split(const std::string& line, char delimiter);
};
