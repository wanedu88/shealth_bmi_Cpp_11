#include "SHealth.h"
#include <fstream>
#include <iostream>
#include <sstream>

using SHealthConstants::kAgeBandCount;
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
    if (!loadRecordsFromFile(filename)) {
        return 0;
    }
    imputeMissingWeightsByAgeBand();
    computeAllBmis();
    aggregateRatiosByAgeBand();
    return recordCount;
}

bool SHealth::loadRecordsFromFile(const std::string& filename) {
    recordCount = 0;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line); // 첫번째 줄 읽기 (헤더)
    while (std::getline(file, line)) {
        if (!parseAndStoreLine(line)) {
            break;
        }
    }
    file.close();
    return true;
}

bool SHealth::parseAndStoreLine(const std::string& line) {
    std::vector<std::string> tokens = split(line, kCsvDelimiter);
    if (tokens.empty()) {
        return false;
    }
    ages[recordCount] = std::stoi(tokens[kCsvColAge]);
    weights[recordCount] = std::stod(tokens[kCsvColWeight]);
    heights[recordCount] = std::stod(tokens[kCsvColHeight]);
    recordCount++;
    return true;
}

bool SHealth::isInAgeBand(int age, int ageBandStart) const {
    return age >= ageBandStart && age < ageBandStart + kAgeBandWidth;
}

int SHealth::ageBandIndexFromStart(int ageBandStart) const {
    return (ageBandStart - kAgeBandStartMin) / kAgeBandStep;
}

int SHealth::ageBandIndexFromClass(int ageClass) const {
    if (ageClass < kAgeBandStartMin || ageClass > kAgeBandStartMax) {
        return -1;
    }
    if ((ageClass - kAgeBandStartMin) % kAgeBandStep != 0) {
        return -1;
    }
    return ageBandIndexFromStart(ageClass);
}

void SHealth::imputeMissingWeightsByAgeBand() {
    for (int ageBandStart = kAgeBandStartMin; ageBandStart <= kAgeBandStartMax;
         ageBandStart += kAgeBandStep) {
        double weightSum = 0;
        int nonZeroWeightCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (isInAgeBand(ages[recordIndex], ageBandStart)) {
                if (weights[recordIndex] == kMissingWeight) {
                    continue;
                }
                weightSum += weights[recordIndex];
                nonZeroWeightCount++;
            }
        }
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (isInAgeBand(ages[recordIndex], ageBandStart) &&
                weights[recordIndex] == kMissingWeight) {
                weights[recordIndex] = weightSum / nonZeroWeightCount;
            }
        }
    }
}

void SHealth::computeAllBmis() {
    for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        const double heightMeters = heights[recordIndex] / kHeightCmPerMeter;
        bmis[recordIndex] = weights[recordIndex] / (heightMeters * heightMeters);
    }
}

SHealth::BmiClassSlot SHealth::classifyBmi(double bmi) const {
    if (bmi <= kBmiUnderMax) {
        return BmiClassSlot::Underweight;
    }
    if (bmi > kBmiUnderMax && bmi < kBmiNormalMax) {
        return BmiClassSlot::Normal;
    }
    if (bmi >= kBmiNormalMax && bmi < kBmiOverweightMax) {
        return BmiClassSlot::Overweight;
    }
    if (bmi >= kBmiOverweightMax) {
        return BmiClassSlot::Obesity;
    }
    return BmiClassSlot::None;
}

void SHealth::aggregateRatiosByAgeBand() {
    for (int bandIndex = 0; bandIndex < kAgeBandCount; bandIndex++) {
        const int ageBandStart = kAgeBandStartMin + bandIndex * kAgeBandStep;
        int underweightCount = 0;
        int normalweightCount = 0;
        int overweightCount = 0;
        int obesityCount = 0;
        int bandMemberCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (isInAgeBand(ages[recordIndex], ageBandStart)) {
                bandMemberCount++;
                switch (classifyBmi(bmis[recordIndex])) {
                    case BmiClassSlot::Underweight:
                        underweightCount++;
                        break;
                    case BmiClassSlot::Normal:
                        normalweightCount++;
                        break;
                    case BmiClassSlot::Overweight:
                        overweightCount++;
                        break;
                    case BmiClassSlot::Obesity:
                        obesityCount++;
                        break;
                    case BmiClassSlot::None:
                        break;
                }
            }
        }
        AgeBandRatios& ratios = ageBandRatios[bandIndex];
        ratios.underweight = (double)underweightCount * kPercentMultiplier / bandMemberCount;
        ratios.normal = (double)normalweightCount * kPercentMultiplier / bandMemberCount;
        ratios.overweight = (double)overweightCount * kPercentMultiplier / bandMemberCount;
        ratios.obesity = (double)obesityCount * kPercentMultiplier / bandMemberCount;
    }
}

double SHealth::ratioForCategory(const AgeBandRatios& ratios, BmiCategoryCode category) const {
    switch (category) {
        case BmiCategoryCode::Underweight:
            return ratios.underweight;
        case BmiCategoryCode::Normal:
            return ratios.normal;
        case BmiCategoryCode::Overweight:
            return ratios.overweight;
        case BmiCategoryCode::Obesity:
            return ratios.obesity;
    }
    return 0.0;
}

double SHealth::getBmiRatio(int ageClass, int type) {
    const int bandIndex = ageBandIndexFromClass(ageClass);
    if (bandIndex < 0) {
        return 0.0;
    }
    switch (static_cast<BmiCategoryCode>(type)) {
        case BmiCategoryCode::Underweight:
        case BmiCategoryCode::Normal:
        case BmiCategoryCode::Overweight:
        case BmiCategoryCode::Obesity:
            return ratioForCategory(ageBandRatios[bandIndex], static_cast<BmiCategoryCode>(type));
        default:
            return 0.0;
    }
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
