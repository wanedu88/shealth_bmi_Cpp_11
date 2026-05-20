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
using SHealthConstants::kCsvColId;
using SHealthConstants::kCsvColHeight;
using SHealthConstants::kCsvColWeight;
using SHealthConstants::kCsvDelimiter;
using SHealthConstants::kHeightCmPerMeter;
using SHealthConstants::kMissingHeight;
using SHealthConstants::kMissingWeight;
using SHealthConstants::kPercentMultiplier;

int SHealth::calculateBmi(const std::string& filename) {
    if (!loadRecordsFromFile(filename)) {
        return 0;
    }
    runBmiPipeline();
    return recordCount;
}

void SHealth::runBmiPipeline() {
    imputeMissingWeightsByAgeBand();
    imputeMissingHeightsByAgeBand();
    computeAllBmis();
    aggregateRatiosByAgeBand();
}

bool SHealth::loadRecordsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    return loadFromStream(file);
}

bool SHealth::loadFromStream(std::istream& input) {
    recordCount = 0;
    std::string line;
    std::getline(input, line);  // header
    while (std::getline(input, line)) {
        if (!parseAndStoreLine(line)) {
            break;
        }
    }
    return true;
}

bool SHealth::parseAndStoreLine(const std::string& line) {
    std::vector<std::string> tokens = split(line, kCsvDelimiter);
    if (tokens.empty()) {
        return false;
    }
    ids[recordCount] = std::stoi(tokens[kCsvColId]);
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

void SHealth::imputeMissingHeightsByAgeBand() {
    for (int ageBandStart = kAgeBandStartMin; ageBandStart <= kAgeBandStartMax;
         ageBandStart += kAgeBandStep) {
        double heightSum = 0;
        int nonZeroHeightCount = 0;
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (isInAgeBand(ages[recordIndex], ageBandStart)) {
                if (heights[recordIndex] == kMissingHeight) {
                    continue;
                }
                heightSum += heights[recordIndex];
                nonZeroHeightCount++;
            }
        }
        if (nonZeroHeightCount == 0) {
            continue;
        }
        for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
            if (isInAgeBand(ages[recordIndex], ageBandStart) &&
                heights[recordIndex] == kMissingHeight) {
                heights[recordIndex] = heightSum / nonZeroHeightCount;
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
    const AgeBandDistribution distribution = getAgeBandDistribution(ageClass);
    switch (static_cast<BmiCategoryCode>(type)) {
        case BmiCategoryCode::Underweight:
            return distribution.underweight;
        case BmiCategoryCode::Normal:
            return distribution.normal;
        case BmiCategoryCode::Overweight:
            return distribution.overweight;
        case BmiCategoryCode::Obesity:
            return distribution.obesity;
        default:
            return 0.0;
    }
}

AgeBandDistribution SHealth::getAgeBandDistribution(int ageClass) const {
    const int bandIndex = ageBandIndexFromClass(ageClass);
    if (bandIndex < 0) {
        return {};
    }
    const AgeBandRatios& ratios = ageBandRatios[bandIndex];
    return {ratios.underweight, ratios.normal, ratios.overweight, ratios.obesity};
}

std::vector<int> SHealth::getNormalBmiUserIds() const {
    std::vector<int> normalIds;
    for (int recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        if (classifyBmi(bmis[recordIndex]) == BmiClassSlot::Normal) {
            normalIds.push_back(ids[recordIndex]);
        }
    }
    return normalIds;
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
