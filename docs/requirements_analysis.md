# SHealth BMI — C++ 구현 관점 요구사항 분석

> **관점**: 시니어 C++ QA 엔지니어  
> **대상**: Shealth C++17 프로젝트 (CMake 3.10+, Google Test v1.14)  
> **근거**: [README.md](./README.md), `src/main/cpp/*`, `src/test/cpp/SHealthBMITest.cpp`

---

## 1) 기능별 규칙 표

| ID | 기능 | 입력 | 처리 규칙 | 출력 | 구현 위치 (현재) | 비고 |
|:---:|------|------|-----------|------|------------------|------|
| F-01 | CSV 데이터 로드 | 파일 경로 (`shealth.dat`) | 헤더 1행 스킵 후 `id,age,weight,height` 파싱 | 레코드 수 (`count`) | `SHealth::calculateBmi` | 파일 미존재 시 `0` 반환, stderr 로그 |
| F-02 | 연령대 구간 정의 | `age` (int) | **10세 단위**: `[20,30)`, `[30,40)`, … `[70,80)` — `age >= a && age < a+10` | 구간 멤버십 | 동일 | 20·30·…·70만 처리; 19세·80세+는 통계·보정 대상 아님 |
| F-03 | 체중 누락 보정 | `weight == 0.0` | 동일 연령대 내 `weight != 0` 레코드의 **산술 평균**으로 대체 | 보정된 `weights[]` | `calculateBmi` 28–47행 | README: “체중 0이 누락” |
| F-04 | BMI 계산 | `weight`(kg), `height`(cm) | `BMI = weight / (height_m)²`, `height_m = height / 100.0` | `bmis[]` | 50–53행 | 부동소수점 `double` |
| F-05 | BMI 분류 | `bmi` (double) | 저체중 `≤18.5`, 정상 `>18.5 && <23`, 과체중 `≥23 && <25`, 비만 `≥25` (README) | 카운트 | 65–73행 | **구현 버그**: 비만 조건이 `>25`라 BMI=25 미분류 |
| F-06 | 연령대별 BMI 비율 | 연령대 시작값 `a` ∈ {20,30,…,70} | 구간 내 인원 대비 각 분류 **백분율** `(count/total)*100` | `underweight*`, `normalweight*`, … | 55–107행 | `sum==0` 시 0으로 나눔 위험 |
| F-07 | 비율 조회 API | `ageClass` (20,30,…), `type` (100/200/300/400) | 매직 넘버로 저장된 비율 반환 | `double` (%) | `getBmiRatio` | 타입 상수·enum 미정의 |
| F-08 | 콘솔 리포트 | — | 20~70대 각 4분류 비율 `printf` | stdout | `SHealthBMI.cpp` | UI·포맷 하드코딩 |

### README 명세 vs 현재 구현 (경계값)

| BMI 값 | README 분류 | 현재 `SHealth.cpp` 분류 | QA 판정 |
|--------|-------------|-------------------------|---------|
| 18.5 | 저체중 (이하) | 저체중 (`<=18.5`) | 일치 |
| 18.5 초과 ~ 23 미만 | 정상 | 정상 (`>18.5 && <23`) | 일치 |
| 23 | 과체중 (이상) | 과체중 (`>=23`) | 일치 |
| 25 | 비만 (이상) | **미분류** (`>25`만 비만) | **불일치 — 결함 후보** |
| 25 초과 | 비만 | 비만 | 일치 |

### 실습 로드맵상 추가 예정 기능 (미구현)

| ID | 기능 | 규칙 (README Activities) |
|:---:|------|---------------------------|
| F-09 | 연령대별 BMI 분포 비율 (개선) | SRP 분리 후 명시적 API |
| F-10 | 키 누락 보정 | `height == 0` → 동 연령대 평균 키 (F-03과 대칭) |
| F-11 | 정상 BMI 사용자 목록 | `18.5 < BMI < 23` 사용자 ID/레코드 반환 |
| F-12 | 전체 대비 분류 비율 | 연령대 무관 전체 인원 기준 4분류 % |

---

## 2) BMI 계산 로직 이해

### 2.1 수식

```
height_m = height_cm / 100.0
BMI      = weight_kg / (height_m * height_m)
```

- 단위: kg, cm → m 변환 후 제곱.
- C++ 타입: `double` 연산 (`weights[]`, `heights[]`, `bmis[]`).

### 2.2 처리 파이프라인 (현재 `calculateBmi` 단일 메서드)

```mermaid
flowchart LR
    A[CSV 로드] --> B[연령대별 weight=0 보정]
    B --> C[BMI 배열 계산]
    C --> D[연령대별 4분류 집계]
    D --> E[백분율 멤버 변수 저장]
```

1. **로드**: `split(line, ',')` → `tokens[1..3]` → age, weight, height.
2. **보정 (2-pass, 연령대별)**  
   - Pass 1: `a ∈ {20,30,…,70}` 구간에서 `weight != 0`만 합산 → `avg = sum / ageCount`.  
   - Pass 2: 같은 구간에서 `weight == 0`인 행에 `avg` 대입.  
   - **리스크**: 구간 내 유효 체중이 0명이면 `ageCount==0` → 0으로 나눔.
3. **BMI**: 보정 후 모든 행에 대해 일괄 계산 (`height==0`이면 inf/NaN 가능 — F-10 전까지 미처리).
4. **통계**: 연령대별로 분류 카운트 후 % 저장.

### 2.3 분류 의사결정 (요구 명세 기준)

| 분류 | 조건 (README) | C++ 표현 예시 (권장) |
|------|---------------|----------------------|
| 저체중 | BMI ≤ 18.5 | `bmi <= 18.5` |
| 정상체중 | 18.5 < BMI < 23 | `bmi > 18.5 && bmi < 23.0` |
| 과체중 | 23 ≤ BMI < 25 | `bmi >= 23.0 && bmi < 25.0` |
| 비만 | BMI ≥ 25 | `bmi >= 25.0` |

### 2.4 샘플 데이터 검증 포인트

| id | age | weight | height | 연령대 | 비고 |
|----|-----|--------|--------|--------|------|
| 93730 | 57 | **0** | 167.6 | 50대 | F-03 보정 대상 |
| 93717 | 22 | 74.4 | 174.1 | 20대 | 정상 분류 후보 |
| 93709 | 75 | 88.8 | 151.1 | **80대 구간 밖** | 20–70 루프에 미포함 |

---

## 3) 기본 코드 구조 정리

### 3.1 빌드·의존성

| 항목 | 값 |
|------|-----|
| CMake | ≥ 3.10 |
| C++ 표준 | C++17 (`CMAKE_CXX_STANDARD 17`) |
| 라이브러리 | `shealth_lib` ← `SHealth.cpp` |
| 실행 파일 | `SHealthBMI` (main), `SHealthBMITest` (GTest) |
| 테스트 | FetchContent → googletest **v1.14.0**, `gtest_discover_tests` |

### 3.2 디렉터리·책임

```
CMakeLists.txt          # 빌드, GTest FetchContent, ctest 등록
shealth.dat               # 통합 테스트용 실데이터 (~4800+ 행)
src/main/cpp/
  SHealth.h / .cpp        # 도메인 전부 (로드·보정·BMI·통계·split)
  SHealthBMI.cpp            # main: calculateBmi + printf 리포트
src/test/cpp/
  SHealthBMITest.cpp        # GTest (현재 FAIL 스텁 1건)
```

### 3.3 클래스 API (현재)

| 구분 | 심볼 | 역할 |
|------|------|------|
| public | `int calculateBmi(const std::string& filename)` | 파일 로드 → 보정 → BMI → 연령대 통계 (부수효과 다수) |
| public | `double getBmiRatio(int ageClass, int type)` | 연령대·분류 코드로 % 조회 |
| private | `split(...)` | CSV 토큰화 |
| private | 고정 크기 배열 `[10000]` | age, height, weight, bmi |
| private | 24개 `double` | 연령대×4분류 비율 캐시 |

### 3.4 QA 관점 코드 스멜 (테스트·리팩터링 우선순위)

| 우선순위 | 이슈 | 테스트·구현 영향 |
|:--------:|------|------------------|
| P0 | `calculateBmi` God Method — I/O·파싱·도메인·집계 혼재 | 단위 테스트 불가에 가까움; 파일/스트림 주입 필요 |
| P0 | BMI=25 미분류 | F-05 경계 TC 필수 |
| P1 | `getBmiRatio` 매직 넘버 (100/200/300/400) | enum class + 명명 상수 |
| P1 | 고정 배열 10000, `count` 무검증 | 버퍼 오버플로, 대용량 CSV |
| P1 | `sum==0`, `ageCount==0` 미처리 | 예외·방어 TC |
| P2 | 24개 유사 멤버 변수 | map/struct 또는 집계 테이블 |
| P2 | `printf` / `std::cerr` 혼용 | main vs 라이브러리 책임 분리 |

### 3.5 테스트 가능 구조 (목표 아키텍처)

리팩터링 후 QA가 기대하는 최소 분리:

| 레이어 | 책임 | 예시 API |
|--------|------|----------|
| Parser | CSV → `UserRecord` | `parseLine`, `loadFromStream` |
| Domain | BMI·분류·보정 | `computeBmi`, `classifyBmi`, `imputeWeightByAgeBand` |
| Statistics | 집계·비율 | `AgeBandStats`, `ratioPercent` |
| App | 파일 경로·출력 | `main` only |

---

## 4) Google Test 기준 테스트 시나리오 목록

> 네이밍 예: `TEST(SHealthBMITest, TC_01_...)`.  
> 현재 `SHealthBMITest.cpp`는 `FailedTest`만 존재 — 아래 시나리오는 **작성 대상**이다.

### 4.1 BMI 계산 (`computeBmi` / 분리 후 단위)

1. 정상 입력: weight=70, height=170 → BMI ≈ 24.22 (허용 오차 `EXPECT_NEAR`, epsilon 1e-2).
2. 키 100cm 경계: height=100 → height_m=1.0, BMI = weight.
3. 소수 체중·키: README 샘플 1행 (79.5, 158.3) 재현.
4. height가 매우 큰 값 — overflow 없이 유한 double 반환.
5. height=0 — 현재는 inf/NaN 또는 미정의; **명세 확정 후** F-10 또는 예외 TC.

### 4.2 체중 보정 (연령대 평균, F-03)

6. 동일 연령대 3명: weights {50, 60, 0} → 0인 행은 55.0으로 보정.
7. 연령대 내 전원 weight=0 — `ageCount==0` 방어 (예외 또는 skip, 요구 확정).
8. 다른 연령대 평균이 섞이지 않음: 20대 평균이 30대 0 레코드에 적용되지 않음.
9. weight=0 단 1명·동대 유효 표본 1명 — 평균=그 1명 체중.
10. 보정 후 BMI가 보정 전(0kg 가정)과 달라짐 — 회귀 방지.

### 4.3 BMI 분류 (F-05, 경계값)

11. BMI=18.5 → 저체중.
12. BMI=18.5000001 → 정상.
13. BMI=22.999… → 정상.
14. BMI=23.0 → 과체중.
15. BMI=24.999… → 과체중.
16. BMI=25.0 → 비만 (README; **현재 구현 실패 예상** — Red → Green).
17. BMI=30.0 → 비만.
18. 분류 상호 배타·완전: 임의 표본에서 정확히 1개 분류.

### 4.4 연령대 통계·비율 (F-06, F-07)

19. 인위적 소규모 CSV(메모리/임시 파일): 20대 4명, 분류 1:1:1:1 → 각 25%.
20. 연령대에 해당 인원 0명 — 비율 0 또는 N/A 정책 검증.
21. `getBmiRatio(20, 100)` == 내부 저체중 % (20대).
22. 잘못된 `ageClass`(19, 80) / `type`(0, 500) — 0.0 또는 에러 정책.
23. `calculateBmi` 호출 전 `getBmiRatio` — 초기값 0.0.

### 4.5 파일·파싱 (F-01)

24. 존재하지 않는 경로 → 반환 0, 레코드 미적재.
25. 헤더만 있는 CSV → count=0.
26. 빈 줄/토큰 부족 — 파싱 중단 또는 skip 정책.
27. `shealth.dat` 실파일 스모크: count > 0, `getBmiRatio` 유한값.
28. id 컬럼 무시 — age/weight/height만 사용해도 동일 결과.

### 4.6 예외·비기능

29. 레코드 수 > 10000 — 오버플로 또는 거부 (요구 정의 필요).
30. 음수 age/weight/height — 거부 또는 abs 정책.
31. 동일 `calculateBmi` 두 번 호출 — 이전 통계 덮어쓰기 일관성.
32. (리팩터링 후) `std::istringstream` 픽스처 — 디스크 없이 4.1~4.4 반복.

### 4.7 실습 로드맵 연계 (F-09~F-12, Green 후 추가)

33. height=0 → 동 연령대 평균 키 보정 (F-10).
34. 정상 BMI 사용자 목록 — 개수·ID 일치 (F-11).
35. 전체 인원 대비 4분류 비율 합 ≈ 100% (F-12).
36. 연령대별 비율 합 ≈ 100% (반올림 허용).

### 4.8 테스트 인프라·실행

37. `ctest` / `SHealthBMITest` — 실패 스텁 `FailedTest` 제거 후 전체 Green.
38. 테스트 데이터: `test/fixtures/minimal.csv` 등 소형 픽스처로 6~22번 결정론 유지.
39. GTest `SetUp`에서 `SHealth` 인스턴스, 경로는 `std::filesystem` (C++17)로 임시 디렉터리.

---

## 부록: 타입·상수 제안 (구현 시)

```cpp
enum class BmiCategory { Underweight, Normal, Overweight, Obese };

constexpr double kBmiUnderMax      = 18.5;
constexpr double kBmiNormalMax     = 23.0;
constexpr double kBmiOverweightMax = 25.0;

// 연령대: [base, base+10)
int ageBandStart(int age);  // 20,30,...,70 or -1
```

---

*문서 생성: README 및 `src/` 현행 코드 기준. 리팩터링·F-09~F-12 반영 시 본 문서의 표·TC 목록을 동기화할 것.*
