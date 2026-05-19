#include "SHealth.h"
#include <cstdio>

int main() {
    SHealth shealth;
    shealth.calculateBmi("shealth.dat");

    const int underweightCode = static_cast<int>(BmiCategoryCode::Underweight);
    const int normalCode = static_cast<int>(BmiCategoryCode::Normal);
    const int overweightCode = static_cast<int>(BmiCategoryCode::Overweight);
    const int obesityCode = static_cast<int>(BmiCategoryCode::Obesity);

    printf("20 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(20, underweightCode), shealth.getBmiRatio(20, normalCode),
           shealth.getBmiRatio(20, overweightCode), shealth.getBmiRatio(20, obesityCode));
    printf("30 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(30, underweightCode), shealth.getBmiRatio(30, normalCode),
           shealth.getBmiRatio(30, overweightCode), shealth.getBmiRatio(30, obesityCode));
    printf("40 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(40, underweightCode), shealth.getBmiRatio(40, normalCode),
           shealth.getBmiRatio(40, overweightCode), shealth.getBmiRatio(40, obesityCode));
    printf("50 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(50, underweightCode), shealth.getBmiRatio(50, normalCode),
           shealth.getBmiRatio(50, overweightCode), shealth.getBmiRatio(50, obesityCode));
    printf("60 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(60, underweightCode), shealth.getBmiRatio(60, normalCode),
           shealth.getBmiRatio(60, overweightCode), shealth.getBmiRatio(60, obesityCode));
    printf("70 - underweight = %f, normal = %f, overweight = %f, obesity = %f\n",
           shealth.getBmiRatio(70, underweightCode), shealth.getBmiRatio(70, normalCode),
           shealth.getBmiRatio(70, overweightCode), shealth.getBmiRatio(70, obesityCode));

    return 0;
}
