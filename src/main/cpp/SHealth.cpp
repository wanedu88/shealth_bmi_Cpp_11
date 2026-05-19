#include "SHealth.h"
#include <fstream>
#include <iostream>
#include <sstream>

using SHealthConstants::kAgeBandStartMax;
using SHealthConstants::kAgeBandStartMin;
using SHealthConstants::kAgeBandStep;
using SHealthConstants::kAgeBandWidth;
using SHealthConstants::kBmiNormalMax;
using SHealthConstants::kBmiOverweightMax;
using SHealthConstants::kBmiUnderMax;
using SHealthConstants::kCsvColAge;
using SHealthConstants::kCsvColHeight;
using SHealthConstants::kCsvColWeight;
using SHealthConstants::kCsvDelimiter;
using SHealthConstants::kHeightCmPerMeter;
using SHealthConstants::kMissingWeight;
using SHealthConstants::kPercentMultiplier;

int SHealth::calculateBmi(const std::string& filename) {
    recordCount = 0;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 0;
    }

    std::string line;
    std::getline(file, line); // 첫번째 줄 읽기 (헤더)
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = split(line, kCsvDelimiter);
        if (tokens.empty()) {
            break;
        }
        ages[recordCount] = std::stoi(tokens[kCsvColAge]);
        weights[recordCount] = std::stod(tokens[kCsvColWeight]);
        heights[recordCount] = std::stod(tokens[kCsvColHeight]);
        recordCount++;
    }
    file.close();

    // 데이터 수집 중 누락된 체중에 나이대의 평균 체중을 적용
    for (int ageBandStart = kAgeBandStartMin; ageBandStart <= kAgeBandStartMax;
         ageBandStart += kAgeBandStep) {
        double weightSum = 0;
        int nonZeroWeightCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + kAgeBandWidth) {
                if (weights[recordIndex] == kMissingWeight) {
                    continue;
                }
                weightSum += weights[recordIndex];
                nonZeroWeightCount++;
            }
        }
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + kAgeBandWidth) {
                if (weights[recordIndex] == kMissingWeight) {
                    weights[recordIndex] = weightSum / nonZeroWeightCount;
                }
            }
        }
    }

    // BMI 계산하기
    for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        const double heightMeters = heights[recordIndex] / kHeightCmPerMeter;
        bmis[recordIndex] = weights[recordIndex] / (heightMeters * heightMeters);
    }

    // 나이대의 BMI기준 저체중, 정상체중, 과체중, 비만 비율 계산
    for (int ageBandStart = kAgeBandStartMin; ageBandStart <= kAgeBandStartMax;
         ageBandStart += kAgeBandStep) {
        int underweightCount = 0;
        int normalweightCount = 0;
        int overweightCount = 0;
        int obesityCount = 0;
        int bandMemberCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + kAgeBandWidth) {
                bandMemberCount++;
                if (bmis[recordIndex] <= kBmiUnderMax) {
                    underweightCount++;
                } else if (bmis[recordIndex] > kBmiUnderMax && bmis[recordIndex] < kBmiNormalMax) {
                    normalweightCount++;
                } else if (bmis[recordIndex] >= kBmiNormalMax && bmis[recordIndex] < kBmiOverweightMax) {
                    overweightCount++;
                } else if (bmis[recordIndex] > kBmiOverweightMax) {
                    obesityCount++;
                }
            }
        }
        if (ageBandStart == kAgeBandStartMin) {
            underweight20 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight20 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight20 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity20 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        } else if (ageBandStart == kAgeBandStartMin + kAgeBandStep) {
            underweight30 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight30 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight30 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity30 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        } else if (ageBandStart == kAgeBandStartMin + 2 * kAgeBandStep) {
            underweight40 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight40 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight40 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity40 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        } else if (ageBandStart == kAgeBandStartMin + 3 * kAgeBandStep) {
            underweight50 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight50 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight50 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity50 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        } else if (ageBandStart == kAgeBandStartMin + 4 * kAgeBandStep) {
            underweight60 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight60 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight60 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity60 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        } else if (ageBandStart == kAgeBandStartMin + 5 * kAgeBandStep) {
            underweight70 = (double)underweightCount * kPercentMultiplier / bandMemberCount;
            normalweight70 = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
            overweight70 = (double)overweightCount * kPercentMultiplier / bandMemberCount;
            obesity70 = (double)obesityCount * kPercentMultiplier / bandMemberCount;
        }
    }
    return recordCount;
}

double SHealth::getBmiRatio(int ageClass, int type) {
    const int categoryCode = type;
    const int underweightCode = static_cast<int>(BmiCategoryCode::Underweight);
    const int normalCode = static_cast<int>(BmiCategoryCode::Normal);
    const int overweightCode = static_cast<int>(BmiCategoryCode::Overweight);
    const int obesityCode = static_cast<int>(BmiCategoryCode::Obesity);

    if (ageClass == kAgeBandStartMin && categoryCode == underweightCode) return underweight20;
    else if (ageClass == kAgeBandStartMin && categoryCode == normalCode) return normalweight20;
    else if (ageClass == kAgeBandStartMin && categoryCode == overweightCode) return overweight20;
    else if (ageClass == kAgeBandStartMin && categoryCode == obesityCode) return obesity20;
    else if (ageClass == kAgeBandStartMin + kAgeBandStep && categoryCode == underweightCode)
        return underweight30;
    else if (ageClass == kAgeBandStartMin + kAgeBandStep && categoryCode == normalCode)
        return normalweight30;
    else if (ageClass == kAgeBandStartMin + kAgeBandStep && categoryCode == overweightCode)
        return overweight30;
    else if (ageClass == kAgeBandStartMin + kAgeBandStep && categoryCode == obesityCode) return obesity30;
    else if (ageClass == kAgeBandStartMin + 2 * kAgeBandStep && categoryCode == underweightCode)
        return underweight40;
    else if (ageClass == kAgeBandStartMin + 2 * kAgeBandStep && categoryCode == normalCode)
        return normalweight40;
    else if (ageClass == kAgeBandStartMin + 2 * kAgeBandStep && categoryCode == overweightCode)
        return overweight40;
    else if (ageClass == kAgeBandStartMin + 2 * kAgeBandStep && categoryCode == obesityCode) return obesity40;
    else if (ageClass == kAgeBandStartMin + 3 * kAgeBandStep && categoryCode == underweightCode)
        return underweight50;
    else if (ageClass == kAgeBandStartMin + 3 * kAgeBandStep && categoryCode == normalCode)
        return normalweight50;
    else if (ageClass == kAgeBandStartMin + 3 * kAgeBandStep && categoryCode == overweightCode)
        return overweight50;
    else if (ageClass == kAgeBandStartMin + 3 * kAgeBandStep && categoryCode == obesityCode) return obesity50;
    else if (ageClass == kAgeBandStartMin + 4 * kAgeBandStep && categoryCode == underweightCode)
        return underweight60;
    else if (ageClass == kAgeBandStartMin + 4 * kAgeBandStep && categoryCode == normalCode)
        return normalweight60;
    else if (ageClass == kAgeBandStartMin + 4 * kAgeBandStep && categoryCode == overweightCode)
        return overweight60;
    else if (ageClass == kAgeBandStartMin + 4 * kAgeBandStep && categoryCode == obesityCode) return obesity60;
    else if (ageClass == kAgeBandStartMin + 5 * kAgeBandStep && categoryCode == underweightCode)
        return underweight70;
    else if (ageClass == kAgeBandStartMin + 5 * kAgeBandStep && categoryCode == normalCode)
        return normalweight70;
    else if (ageClass == kAgeBandStartMin + 5 * kAgeBandStep && categoryCode == overweightCode)
        return overweight70;
    else if (ageClass == kAgeBandStartMin + 5 * kAgeBandStep && categoryCode == obesityCode) return obesity70;
    return 0.0;
}

std::vector<std::string> SHealth::split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
