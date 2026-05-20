# SHealth BMI — 결함 목록 (defect_list)

| 항목 | 내용 |
|------|------|
| 문서 버전 | 1.0 |
| 작성일 | 2026-05-20 |
| 기준 ctest | **25/26 Green** (96%), 실패 **1건** |
| SSOT | [docs/test_plan.md](./test_plan.md) §8, [docs/requirements_analysis.md](./requirements_analysis.md) §4 |
| 실패 로그 | `build/Testing/Temporary/LastTest.log`, `LastTestsFailed.log` |

---

## 1. 요약

| 구분 | 건수 | 비고 |
|------|------|------|
| **Open P0** (ctest 실패) | **1** | DEF-001 |
| P1 (스냅샷·명세 미확정) | 1 | DEF-002 |
| P2 (기능 개선 범위) | 1 | DEF-003 |
| 연쇄·모니터링 | 1 | DEF-004 (현재 Green, 수정 후 회귀 점검) |

**우선 수정 순서:** DEF-001 → DEF-004 회귀 확인 → (별도 턴) DEF-002·003 요구 확정

---

## 2. 결함 마스터

| DEF-ID | TC | TEST_F | 심각도 | 상태 | 요구/F-ID | 증상 | 근본 원인 (파일:위치) | 최소 수정 | 영향 TC | 검증 |
|--------|-----|--------|--------|------|-----------|------|----------------------|-----------|---------|------|
| **DEF-001** | 16 | `TC_16_Boundary_Obesity_25` | **P0** | **Confirmed (Red)** | F-05 | 비만(400) 비율 기대 100% → **0%** | `SHealth.cpp:121` `classifyBmi` — 비만 조건 `bmi > kBmiOverweightMax` (25.0 **제외**) | `bmi >= kBmiOverweightMax` 로 1줄 변경; TC_16 Green 시 픽스처 `72.25` 검토(§3) | 16, (18) | `ctest` TC_16, 전체 26/26 |
| DEF-002 | 07 | `TC_07_AllWeightsZero_DivideByZero` | P1 | Snapshot | F-03 | 연령대 전원 weight=0 시 `weightSum/0` → NaN 보정 | `SHealth.cpp:98` `imputeMissingWeightsByAgeBand` — `nonZeroWeightCount==0` 미가드 | `if (nonZeroWeightCount == 0) continue;` 또는 평균 skip (요구 확정 후) | 06~10 | TC_07 기대값 재정의 후 Green |
| DEF-003 | 05 | `TC_05_HeightZero_CurrentBehavior` | P2 | Snapshot | F-10 (README §4) | height=0 → BMI `inf` → `bmi>25`로 **비만 100%** | `SHealth.cpp:104-107` `computeAllBmis` — 0 나눗셈 미처리 | README 4단계: height=0 보정 로직 (범위 외·별도 스프린트) | 05 | F-10 구현 TC 추가 |
| DEF-004 | 18 | `TC_18_Classification_ExclusiveComplete` | 연쇄 | Monitor (Green) | F-05, F-06 | 4분류 합 **100%** 미달 가능 | DEF-001 시 `BmiClassSlot::None` 누락분 | DEF-001 수정 후 `ratioSumForBand` 재검증 | 18 | TC_18 + 합≈100 |

---

## 3. 상세 — P0

### DEF-001 — BMI 경계 25.0 비만 미적용

#### 실패 로그 발췌

```
SHealthBMITest.TC_16_Boundary_Obesity_25
C:\DEV\shealth_bmi_Cpp_11\src\test\cpp\SHealthBMITest.cpp:124: Failure
The difference between 100.0 and health.getBmiRatio(ageClass, static_cast<int>(category)) is 100
  100.0  → 기대
  health.getBmiRatio(...) → 0  (비만 코드 400)
```

#### Given–When–Then

| 단계 | 내용 |
|------|------|
| Given | `test/fixtures/tc16_bmi_25.csv` — age 25, weight **72.249**, height 170 |
| When | `calculateBmi(path)` → `aggregateRatiosByAgeBand()` |
| Then (README) | 20대 **비만(400) 100%** |
| Then (실제) | 비만 0%; 해당 레코드는 **과체중(300)** 으로 집계됨 |

#### 수치 추적

- BMI = `72.249 / (1.7)²` ≈ **24.9997** (&lt; 25.0)
- `classifyBmi`: `23 ≤ bmi < 25` → `BmiClassSlot::Overweight` (L118–119)
- README·`.cursorrules`: **BMI ≥ 25 → 비만**

추가로, `bmi == 25.0` 정확히일 때 현재 코드는 L121 조건 `bmi > 25` 불만족, L118도 불만족 → **`BmiClassSlot::None`** (분류 공백). 이는 명세상 비만이어야 함.

#### 버그 위치

```111:124:src/main/cpp/SHealth.cpp
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
    if (bmi > kBmiOverweightMax) {   // ← DEF-001: `>` 는 25.0 제외
        return BmiClassSlot::Obesity;
    }
    return BmiClassSlot::None;
}
```

상수 정의는 README와 일치 (`kBmiOverweightMax = 25.0`).

```8:10:src/main/cpp/SHealth.h
constexpr double kBmiUnderMax = 18.5;        // 저체중: BMI ≤ 18.5
constexpr double kBmiNormalMax = 23.0;         // 정상 상한(미만): 18.5 초과 ~ 23 미만
constexpr double kBmiOverweightMax = 25.0;   // 과체중 상한(미만): 23 이상 ~ 25 미만
```

#### 최소 수정 방안 (제안·미적용)

**1) 프로덕션 코드 (필수, 1줄)**

```cpp
// Before (L121)
if (bmi > kBmiOverweightMax) {

// After
if (bmi >= kBmiOverweightMax) {
```

**2) TC_16 Green 검증 시 픽스처 (조건부)**

| weight | BMI (h=170) | `>=` 수정만 | README «BMI=25.0 비만» |
|--------|-------------|-------------|-------------------------|
| 72.249 | ≈24.9997 | 과체중 유지 | **불일치** — TC_16 계속 Red 가능 |
| **72.25** | **25.0** | **비만** | **일치** |

`test_plan.md` §8 Green 턴: 코드 수정 + `tc16_bmi_25.csv` weight **72.25** 권장.

#### SSOT 정합

| 문서 | BMI=25.0 |
|------|----------|
| README / `.cursorrules` / `test_plan` §1.4 | **비만 (≥25)** |
| 현재 `SHealth.cpp` | `>25` 만 비만; `==25` → **None** |

**결함 SSOT:** README·`test_plan` — 구현이 미준수.

---

## 4. 스냅샷·연쇄 (수정 보류 / 모니터링)

### DEF-002 — 연령대 전원 weight=0 (P1, Snapshot)

| 항목 | 내용 |
|------|------|
| TC | 07 — `TC_07_AllWeightsZero_DivideByZero` |
| 현재 테스트 | ctest **Green** (스냅샷: 4분류 0%, 합 0%) |
| 코드 | `imputeMissingWeightsByAgeBand` L98: `weights[recordIndex] = weightSum / nonZeroWeightCount` — 분모 0 |
| `test_plan` §8 | 요구 확정 후 Green; 당장은 **동작 고정** |
| 최소 수정 (안) | `nonZeroWeightCount == 0` 이면 보정 루프 skip |

### DEF-003 — height=0 (P2, Snapshot)

| 항목 | 내용 |
|------|------|
| TC | 05 — `TC_05_HeightZero_CurrentBehavior` |
| 현재 테스트 | ctest **Green** (스냅샷: inf → 비만 100%) |
| README | §4 F-10 — **기능 개선** 단계 (미구현) |
| 최소 수정 | 별도 스프린트; 3단계 단위 테스트 범위 **Out of Scope** |

### DEF-004 — TC_18 비율 합 (연쇄, Monitor)

| 항목 | 내용 |
|------|------|
| TC | 18 — `TC_18_Classification_ExclusiveComplete` |
| 현재 | ctest **Green** (각 25%, 합 100%) |
| 위험 | DEF-001로 `None` 레코드 발생 시 합 &lt; 100% |
| 조치 | DEF-001 적용 후 **TC_18·TC_16** 동시 회귀 |

---

## 5. 수정 우선순위 · Green 체크리스트

| 순서 | DEF-ID | 작업 | 완료 기준 |
|:----:|--------|------|-----------|
| 1 | DEF-001 | `classifyBmi` L121 `>=` | TC_16 Green |
| 2 | DEF-001 | (필요 시) `tc16_bmi_25.csv` weight → 72.25 | README BMI=25.0 |
| 3 | DEF-004 | 회귀 | TC_18·전체 `ctest` 26/26 |
| 4 | — | `test_plan.md` §3 TC_16 상태, §8 Red 해소 | 문서 동기화 |
| 5 | DEF-002·003 | 요구·README §4 확정 후 | 별도 결함 턴 |

### 권장 구현 프롬프트 (다음 턴)

```
[P] 시니어 C++ QA (TDD Green)
[C] docs/defect_list.md DEF-001
[T] SHealth::classifyBmi L121만 bmi >= kBmiOverweightMax.
    필요 시 tc16 weight 72.25. TC_16·TC_18·ctest 26/26 Green.
[F] diff + ctest + defect_list DEF-001 Fixed + test_plan §8 갱신
금지: DEF-002·003, 그 외 리팩토링
```

---

## 6. ctest 실행 기록

```
Test project C:/DEV/shealth_bmi_Cpp_11/build
96% tests passed, 1 tests failed out of 26
FAILED: 17 - SHealthBMITest.TC_16_Boundary_Obesity_25
```

| # | 결과 | TEST_F |
|---|------|--------|
| 1–16, 18–26 | Passed | (전체 목록은 LastTest.log 참고) |
| **17** | **Failed** | `TC_16_Boundary_Obesity_25` |

---

*본 문서는 ctest 실패 로그 기반 결함 분석 산출물이며, 프로덕션·테스트 코드는 아직 수정하지 않았습니다.*
