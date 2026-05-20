# SHealth BMI — 회고 및 발표 (5단계)

| 항목 | 내용 |
|------|------|
| 프로젝트 | SHealth BMI (삼성 헬스 연령대별 BMI 통계) |
| 기술 스택 | C++17, CMake 3.10+, Google Test v1.14 |
| 작성일 | 2026-05-20 |
| 보고 범위 | README **5. 회고 및 발표** (89~94행) 전 항목 |
| 관점 | 시니어 C++ QA / 기술 발표 |
| 사실 근거 | [02](./02_요구사항분석.md)~[09](./09_기능개선.md), [defect_list](../docs/defect_list.md), [test_plan](../docs/test_plan.md) |
| 선행 스냅샷 | [01_실습보고서.md](./01_실습보고서.md) — 1단계 초기본(본 문서로 5단계 최종본 대체·연계) |
| Git | `feature` @ `0703ead` |

---

## 요약

6시간 실습에서 **분석 → 1차 리팩토링 → 단위 테스트(TDD) → 기능 개선 → 회고** 순서를 `.cursorrules`와 README Activities에 맞춰 완료했다. 최종 **`ctest` 31/31 Green** (100%), README 1~4단계 **19/19 [x]**, P0 결함 **DEF-001·DEF-003 Fixed** (Report 08·09, defect_list v1.2). 잔여 리스크는 **DEF-002(TC_07 스냅샷)**·**TC_32 로드맵** 정도이다.

---

## 1. 실습 목표와 달성도

### 1.1 README Activities 체크리스트 (64~94행)

| 단계 | 항목 | 상태 |
|------|------|:----:|
| **1** | 기본 코드구조, BMI 로직 이해 | [x] |
| **1** | 코드 스멜 찾기 | [x] |
| **2** | 네이밍 개선 | [x] |
| **2** | 하드코드 및 전역변수 제거 | [x] |
| **2** | 함수 추출 | [x] |
| **2** | 반복/중복 제거 | [x] |
| **3** | BMI 계산 로직 TC | [x] |
| **3** | Age 평균치 보정 로직 TC | [x] |
| **3** | 정상/저체중/과체중/비만 분류 TC | [x] |
| **3** | 예외상황 TC | [x] |
| **4** | SRP에 따른 책임 분리등 리팩토링 | [x] |
| **4** | 특정 연령대의 BMI 분포 비율 계산 기능 추가 | [x] |
| **4** | Height가 0인 경우에 대한 평균치 보정 로직 추가 | [x] |
| **4** | BMI 정상 범위 사용자 목록 조회 기능 추가 | [x] |
| **4** | 전체 사용자 대비 각 BMI 범주 비율 계산 기능 추가 | [x] |
| **5** | 실습 목표와 달성도 | [x] |
| **5** | 코드 품질 Before & After | [x] |
| **5** | AI를 어떻게 활용했나? 도움이 된 순간과 한계는? | [x] |
| **5** | TC를 추가보면서 개선에 미친 영향, TC 작성 팁 | [x] |
| **5** | 클린코드와 리팩토링에서 느낀 장점과 어려운점 | [x] |

**종합:** Activities **24/24 [x]** (1~5단계). 5단계 본문은 본 문서로 충족.

### 1.2 단계별 한 줄 성과 · 미달/리스크

| 단계 | 성과 | 미달/리스크 |
|------|------|-------------|
| **1** 분석 | SOLID·스멜·God Method 로드맵·요구 TC 목록 확정 (Report 02·03, requirements_analysis §4) | — |
| **2** 1차 리팩토링 | 네이밍·상수화·private 추출·DRY 4/4, `./SHealthBMI` 기준선 유지 (Report 04, `refactoring` `03ddafb`~`f8484e0`) | 의도적 미수정: BMI≥25·0 나눗셈 → 3단계로 이관 (Report 04 §1.2) |
| **3** 단위 테스트 | 26 `TEST_F`, TC_16 Red → DEF-001 Green **26/26** (Report 05·06·07·08, `tc` `28142cb`) | **DEF-002** TC_07 전원 weight=0 — Snapshot, 요구 확정 대기 (defect_list) |
| **4** 기능 개선 | SRP·F-09~F-12, **ctest 31/31**, DEF-003 Fixed (Report 09, `feature` `a1a887a`~`b3761ad`) | **TC_32** `istream` 테스트·대규모 `vector` 전환은 로드맵 (Report 09 §9) |
| **5** 회고·발표 | 본 문서 + 발표 부록 §6 | — |

---

## 2. 코드 품질 Before & After

### 2.1 Before (Report 03 기준)

- **`calculateBmi` God Method** (~100줄): I/O·파싱·체중 보정·BMI·24멤버 집계가 한 메서드에 결합 — 단위 TC 곤란 (Report 03 §1·§3.1).
- **SRP·DIP 위반**: `ifstream`/`cerr`/`printf` 직접 결합, Parser·Domain·Statistics 레이어 부재 (Report 03 §1.1).
- **매직 넘버·Primitive Obsession**: BMI 18.5/23/25, 연령대 20~70, `getBmiRatio` 코드 100~400 (Report 03 §2).
- **중복 분류·연령대 if**: `classifyBmi` 미추출·6벌 if-else (Report 03 §2.1 P0).
- **도메인 버그**: 비만 `bmi>25` vs README `≥25` — TC_16 Red 원인 (Report 03 §2.1, DEF-001).
- **0 나눗셈·height=0**: `sum/ageCount`, height=0 시 `inf` (Report 03 §2.1 P0/P1) — 후속 DEF-002·DEF-003.
- **테스트**: `FAIL()` 스텁만 존재 (Report 03 §2.1, 02 요약).

### 2.2 After (Report 04·09, `SHealth.h` 기준)

- **Facade + 파이프라인**: `calculateBmi` → `loadRecordsFromFile` / `loadFromStream` / `runBmiPipeline` (Report 09 §3.1).
- **상수·타입**: `SHealthConstants`, `BmiCategoryCode`, `AgeBandDistribution`, `OverallBmiDistribution` (`SHealth.h` L7~53).
- **public API 5종**: `calculateBmi`, `getBmiRatio`, `getAgeBandDistribution`(F-09), `getNormalBmiUserIds`(F-11), `getOverallBmiDistribution`(F-12) (Report 09 §2.2).
- **분류 단일화**: `classifyBmi` `>= kBmiOverweightMax` — DEF-001 Fixed (Report 08, defect_list).
- **height 보정**: `imputeMissingHeightsByAgeBand` — DEF-003 Fixed (Report 09 §3.3, TC_34).
- **검증**: **`ctest` 31/31 Green**, Open P0 **0** (Report 09 §1.3, 로컬 `build/` 2026-05-20).

### 2.3 비교 표

| 항목 | Before | After | 근거 Report |
|------|--------|-------|-------------|
| 구조 | God Method 1개 (~100줄) | Facade + Parser + `runBmiPipeline` | 03 §3.1 → 09 §3.1 |
| SOLID-S | I/O·도메인·통계 혼재 | 로드/파싱/파이프라인 분리 | 03 §1 → 09 §3.1 |
| SOLID-D | 파일 직접 결합 | `loadFromStream(istream&)` (DIP 준비) | 03 §1.1 → 09 §3.1 |
| 상수·타입 | 매직 넘버·`type` 100~400 | `SHealthConstants`, `BmiCategoryCode` | 03 §2 → 04 §2.2 |
| 분류 버그 | `>25` → BMI=25 미분류 | `>=25`, TC_16 Green | 03 §2.1 → 08, defect_list DEF-001 |
| height=0 | BMI `inf` 스냅샷 | 연령대 평균 키 보정 | 03 P1 → 09 §3.3, DEF-003 |
| 테스트 | `FAIL()` 스텁 | 31 `TEST_F`, 31/31 Green | 03 §2.1 → 06·09 §1.3 |
| 기능 | 연령대 `getBmiRatio`만 | F-09~F-12 API + main 3종 출력 | 02 F-01~08 → 09 §2.2 |

---

## 3. 생성형 AI 활용 회고

### 3.1 도움이 된 순간 (3)

1. **PCTF + `[금지]` 범위 통제** — 2단계에서 “네이밍만”“상수화만”으로 턴을 쪼개 `./SHealthBMI` 기준선을 유지했다 (Prompting/04, Report 04 §2). 4단계도 “F-09만”“DEF-001 재수정 금지”로 회귀를 줄였다 (Prompting/09, Report 09 §1.2).
2. **Red → 결함 분석 → Green** — TC_16 Red(25/26) 후 `defect_list`·Report 07로 근본 원인을 고정하고, 최소 수정(`>=`, 픽스처 72.25)으로 **26/26** 달성 (Report 06·07·08, DEF-001).
3. **SSOT 문서 연계** — `@docs/test_plan.md`·`@docs/defect_list.md`·Report 번호를 프롬프트에 고정해 AI 출력과 사람 검토 기준을 맞췄다 (`.cursorrules`, Report 05·07).

### 3.2 한계·주의 (3)

1. **경계값·README 해석** — BMI=25를 코드 `>`와 README `≥`가 어긋나 TC_16이 Red가 됐다. 도메인 표를 프롬프트에 명시하지 않으면 AI가 “기존 동작 보존”으로 버그를 정당화할 수 있다 (DEF-001, test_plan §6 TC_16).
2. **한 턴 과다 변경** — God Method 전면 `vector`·다중 파일 동시 수정은 `.cursorrules` 진행 순서와 충돌한다. 단계·브랜치(`refactoring` / `tc` / `feature`) 분리가 필요했다 (Report 04 §1.2, 09 §1.2).
3. **스냅샷 TC 오해** — TC_07(DEF-002)처럼 “현재 동작 기록”과 “버그 수정”을 구분하지 않으면 Green만 맞추고 요구는 방치할 수 있다 (defect_list DEF-002 P1 Snapshot).

### 3.3 재사용 프롬프트 패턴 (Prompting/04·09)

**패턴 A — 2단계 리팩토링 (한 축만)**

```
[P] 시니어 C++ 리팩토링 엔지니어
[C] SHealth BMI C++17 (.cursorrules 2단계: 리팩토링만)
[T] **네이밍만** 개선. 로직·숫자·분기·public API 시그니처 변경 금지.
    금지: BMI>=25, 가드, 함수 추출, README 4단계
[F] 변경 표 + diff + ./SHealthBMI 기준선 체크리스트
```
(Prompting/04 L30~36)

**패턴 B — 4단계 기능 (한 F-ID만)**

```
[P] F-09 연령대 BMI 분포 비율만 구현
[C] @docs/requirements_analysis.md @docs/test_plan.md, ctest 26/26 유지
[T] getAgeBandDistribution + TC_33. 금지: F-10~12, DEF-001 재수정, God Method 전면 재작성
[F] API 시그니처 표 + TC_33 Green + ctest 결과
```
(Prompting/09 L77~78 요지)

---

## 4. 단위 테스트가 개선에 미친 영향 + TC 작성 팁

### 4.1 결함·기능과 TC 연결

| 흐름 | TC | 결함/기능 | 결과 |
|------|-----|-----------|------|
| 분류 경계 Red | **TC_16** | **DEF-001** P0 — `classifyBmi` `>` vs `≥` | Green: `>=` + `tc16` weight 72.25 → **26/26** (Report 08, defect_list) |
| 연쇄 회귀 | **TC_18** | **DEF-004** — 4분류 합 100% | DEF-001 수정 후 Verified (Report 08 §1.1, defect_list) |
| height 보정 | **TC_05**, **TC_34** | **DEF-003** P2 — F-10 | `imputeMissingHeightsByAgeBand` → **28/28** 문서 기록 후 **31/31** (Report 09, defect_list) |
| 4단계 API | **TC_33~37** | F-09~F-12 | 연령대·전체·정상 목록 회귀 고정 (Report 09 §3.2~3.4, TC_36·37) |

**영향 요약:** 테스트가 **명세(README)·코드 불일치를 수치로 드러냈고**, 리팩토링·기능 추가 후에도 **31/31**로 회귀 방어막이 됐다 (Report 06 Red 1건 → 09 31건).

### 4.2 TC 작성 팁 (5)

- **Given-When-Then 주석**을 `TEST_F` 본문 상단에 고정한다 — `.cursorrules` 테스트 규칙, Report 05 §2.
- **경계값은 README 표와 쌍으로** — 18.5/23/25는 TC_11~17 + 픽스처 CSV에 기대 BMI·weight를 적어 둔다 (test_plan §6, DEF-001).
- **픽스처 CSV는 `test/fixtures/tcNN_*.csv`**, `SHEALTH_TEST_FIXTURE_DIR`로 경로를 빌드에 주입한다 (Report 06 §2.1).
- **한 TC = 한 실패 원인** — TC_16만 Red로 두고 결함 ID를 붙이면 Green 턴 범위가 좁아진다 (Report 07, 08).
- **스냅샷 vs 수정 의도를 TC 이름·defect_list에 명시** — TC_07은 Snapshot(DEF-002), TC_05는 F-10 이후 동작 변경(TC_34)으로 구분한다 (defect_list, Report 09).

---

## 5. 클린코드·리팩토링 회고

### 5.1 장점 (3)

1. **기준선 출력으로 안전망** — 2단계마다 `./SHealthBMI` 6연령대 수치를 비교해 “리팩터인지 버그인지”를 분리했다 (Report 04 요약, Prompting/04).
2. **작은 private 추출 → SRP** — 한 번에 클래스를 쪼개지 않고 `classifyBmi`·`runBmiPipeline` 순으로 쌓아 4단계 SRP가 자연스럽게 이어졌다 (Report 04 §2 → 09 §3.1).
3. **테이블·enum으로 중복 제거** — 24멤버 if-else를 `AgeBandRatios[6]`·`BmiCategoryCode`로 줄여 F-09~12 추가 시 조회 API만 얹으면 됐다 (Report 04 §2.4, 09 §2.2).

### 5.2 어려운 점 (3)

1. **God Method 잔재** — C 스타일 고정 배열 `ids[10000]` 등은 남아 있어 OCP·테스트 격리에 한계가 있다 (Report 03 §1.1, `SHealth.h` L79~84).
2. **단계 순서와 욕구 충돌** — 버그(BMI≥25)를 리팩토링 중 고치고 싶지만 `.cursorrules`는 **테스트 후 수정**을 강제해 TC_16 Red 기간이 필요했다 (Report 04 §1.2, 06).
3. **public API 레거시 유지** — `getBmiRatio(ageClass, type)` 매직 코드를 유지한 채 신규 struct API를 병행해 표면이 이중화됐다 (Report 09 §3.2).

### 5.3 다음에 하면 좋을 것 (2)

1. **DEF-002 요구 확정 후 TC_07 Green** — `nonZeroWeightCount==0` 가드 또는 skip 정책 확정 (defect_list, test_plan TC_07).
2. **TC_32 + `istream` 픽스처** — `loadFromStream` 단독 검증으로 디스크 없는 Parser TC 완성 (Report 09 §9, code_quality_report DIP 로드맵).

---

## 6. 발표용 요약 (부록)

### 6.1 5분 발표 — 슬라이드 5장

**슬라이드 1 — 문제·목표**

- 삼성 헬스: 연령대별 BMI 4분류 통계
- 레거시 C++: God Method·품질 이슈 다수
- 6시간: 분석 → 리팩토링 → UT → 기능 → 회고 (README Activities)

**슬라이드 2 — Before: 무엇이 나빴나**

- `calculateBmi`에 I/O~집계 일괄 (Report 03)
- BMI=25 미분류·중복 if·`FAIL()` 스텁 (DEF-001, Report 03 §2.1)
- SOLID-S/DIP 위반

**슬라이드 3 — 테스트가 바꾼 것**

- 26 `TEST_F` 구현 → TC_16 **Red** (Report 06)
- 결함 분석 → DEF-001 **Green 26/26** (Report 07·08)
- DEF-003·F-09~12 → **31/31** (Report 09)

**슬라이드 4 — After: 구조·API**

- `loadFromStream` + `runBmiPipeline` Facade (Report 09 §3.1)
- F-09~F-12 public API 3종 추가
- `SHealthConstants`·enum·struct로 가독성

**슬라이드 5 — AI 활용·교훈**

- PCTF·`[금지]`·한 턴 한 축 (Prompting/04·09)
- SSOT: test_plan / defect_list / Report
- 남은 과제: DEF-002, TC_32

### 6.2 데모 시나리오

```bash
cd build
./SHealthBMI
```

```bash
cd build
ctest --output-on-failure
```

(기대: 연령대·전체·정상 BMI 목록 출력 — Report 09 요약; **31 passed, 0 failed** — Report 09 §1.3)

---

## 참고

| 문서 | 용도 |
|------|------|
| [01_실습보고서.md](./01_실습보고서.md) | 1단계 초기 스냅샷(과거 진행률 — 본 문서가 5단계 최종) |
| [02_요구사항분석.md](./02_요구사항분석.md) ~ [09_기능개선.md](./09_기능개선.md) | 단계별 SSOT 요약 |
| [docs/defect_list.md](../docs/defect_list.md) | DEF-001~004 |
| [docs/test_plan.md](../docs/test_plan.md) | TC 01~31·33~37 |
| [Prompting/04_1차리팩토링.md](../Prompting/04_1차리팩토링.md) ~ [09_기능개선.md](../Prompting/09_기능개선.md) | 세션 프롬프트 아카이브 |
| [Prompting/10_회고및발표.md](../Prompting/10_회고및발표.md) | 5단계 회고·비기너 프롬프트 아카이브 |

---

*작성 기준: `feature` @ `0703ead`, 로컬 `ctest` 31/31 Green (2026-05-20). [01_실습보고서.md](./01_실습보고서.md)는 수정하지 않음.*
