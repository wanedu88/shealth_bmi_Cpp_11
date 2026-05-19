#include "SHealth.h"
#include <fstream>
#include <iostream>
#include <sstream>

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
        std::vector<std::string> tokens = split(line, ',');
        if (tokens.empty()) {
            break;
        }
        ages[recordCount] = std::stoi(tokens[1]);
        weights[recordCount] = std::stod(tokens[2]);
        heights[recordCount] = std::stod(tokens[3]);
        recordCount++;
    }
    file.close();

    // 데이터 수집 중 누락된 체중에 나이대의 평균 체중을 적용
    for (int ageBandStart = 20; ageBandStart <= 70; ageBandStart += 10) {
        double weightSum = 0;
        int nonZeroWeightCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + 10) {
                if (weights[recordIndex] == 0.0) {
                    continue;
                }
                weightSum += weights[recordIndex];
                nonZeroWeightCount++;
            }
        }
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + 10) {
                if (weights[recordIndex] == 0.0) {
                    weights[recordIndex] = weightSum / nonZeroWeightCount;
                }
            }
        }
    }

    // BMI 계산하기
    for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        bmis[recordIndex] = weights[recordIndex] /
                            ((heights[recordIndex] / 100.0) * (heights[recordIndex] / 100.0));
    }

    // 나이대의 BMI기준 저체중, 정상체중, 과체중, 비만 비율 계산
    for (int ageBandStart = 20; ageBandStart <= 70; ageBandStart += 10) {
        int underweightCount = 0;
        int normalweightCount = 0;
        int overweightCount = 0;
        int obesityCount = 0;
        int bandMemberCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (ages[recordIndex] >= ageBandStart && ages[recordIndex] < ageBandStart + 10) {
                bandMemberCount++;
                if (bmis[recordIndex] <= 18.5) {
                    underweightCount++;
                } else if (bmis[recordIndex] > 18.5 && bmis[recordIndex] < 23) {
                    normalweightCount++;
                } else if (bmis[recordIndex] >= 23 && bmis[recordIndex] < 25) {
                    overweightCount++;
                } else if (bmis[recordIndex] > 25) {
                    obesityCount++;
                }
            }
        }
        if (ageBandStart == 20) {
            underweight20 = (double)underweightCount * 100 / bandMemberCount;
            normalweight20 = (double)normalweightCount * 100 / bandMemberCount;
            overweight20 = (double)overweightCount * 100 / bandMemberCount;
            obesity20 = (double)obesityCount * 100 / bandMemberCount;
        } else if (ageBandStart == 30) {
            underweight30 = (double)underweightCount * 100 / bandMemberCount;
            normalweight30 = (double)normalweightCount * 100 / bandMemberCount;
            overweight30 = (double)overweightCount * 100 / bandMemberCount;
            obesity30 = (double)obesityCount * 100 / bandMemberCount;
        } else if (ageBandStart == 40) {
            underweight40 = (double)underweightCount * 100 / bandMemberCount;
            normalweight40 = (double)normalweightCount * 100 / bandMemberCount;
            overweight40 = (double)overweightCount * 100 / bandMemberCount;
            obesity40 = (double)obesityCount * 100 / bandMemberCount;
        } else if (ageBandStart == 50) {
            underweight50 = (double)underweightCount * 100 / bandMemberCount;
            normalweight50 = (double)normalweightCount * 100 / bandMemberCount;
            overweight50 = (double)overweightCount * 100 / bandMemberCount;
            obesity50 = (double)obesityCount * 100 / bandMemberCount;
        } else if (ageBandStart == 60) {
            underweight60 = (double)underweightCount * 100 / bandMemberCount;
            normalweight60 = (double)normalweightCount * 100 / bandMemberCount;
            overweight60 = (double)overweightCount * 100 / bandMemberCount;
            obesity60 = (double)obesityCount * 100 / bandMemberCount;
        } else if (ageBandStart == 70) {
            underweight70 = (double)underweightCount * 100 / bandMemberCount;
            normalweight70 = (double)normalweightCount * 100 / bandMemberCount;
            overweight70 = (double)overweightCount * 100 / bandMemberCount;
            obesity70 = (double)obesityCount * 100 / bandMemberCount;
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

    if (ageClass == 20 && categoryCode == underweightCode) return underweight20;
    else if (ageClass == 20 && categoryCode == normalCode) return normalweight20;
    else if (ageClass == 20 && categoryCode == overweightCode) return overweight20;
    else if (ageClass == 20 && categoryCode == obesityCode) return obesity20;
    else if (ageClass == 30 && categoryCode == underweightCode) return underweight30;
    else if (ageClass == 30 && categoryCode == normalCode) return normalweight30;
    else if (ageClass == 30 && categoryCode == overweightCode) return overweight30;
    else if (ageClass == 30 && categoryCode == obesityCode) return obesity30;
    else if (ageClass == 40 && categoryCode == underweightCode) return underweight40;
    else if (ageClass == 40 && categoryCode == normalCode) return normalweight40;
    else if (ageClass == 40 && categoryCode == overweightCode) return overweight40;
    else if (ageClass == 40 && categoryCode == obesityCode) return obesity40;
    else if (ageClass == 50 && categoryCode == underweightCode) return underweight50;
    else if (ageClass == 50 && categoryCode == normalCode) return normalweight50;
    else if (ageClass == 50 && categoryCode == overweightCode) return overweight50;
    else if (ageClass == 50 && categoryCode == obesityCode) return obesity50;
    else if (ageClass == 60 && categoryCode == underweightCode) return underweight60;
    else if (ageClass == 60 && categoryCode == normalCode) return normalweight60;
    else if (ageClass == 60 && categoryCode == overweightCode) return overweight60;
    else if (ageClass == 60 && categoryCode == obesityCode) return obesity60;
    else if (ageClass == 70 && categoryCode == underweightCode) return underweight70;
    else if (ageClass == 70 && categoryCode == normalCode) return normalweight70;
    else if (ageClass == 70 && categoryCode == overweightCode) return overweight70;
    else if (ageClass == 70 && categoryCode == obesityCode) return obesity70;
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
