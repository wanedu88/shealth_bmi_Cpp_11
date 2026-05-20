# Report

SHealth BMI C++17 실습 산출물·회고 문서 디렉터리입니다.

| 파일 | 설명 |
|------|------|
| [01_실습보고서.md](./01_실습보고서.md) | 1·5단계 회고(목표·달성도, Before/After, AI 활용, TC 전략, 클린코드) |
| [02_요구사항분석.md](./02_요구사항분석.md) | C++ QA 관점 요구사항(기능 규칙, BMI 로직, 구조, GTest TC 1~39) |
| [03_코드품질분석.md](./03_코드품질분석.md) | SOLID·Code Smell, God Method 분해, 도메인 불일치, 1차 리팩토링 로드맵 |
| [04_1차리팩토링.md](./04_1차리팩토링.md) | 2단계 클린코드 리팩토링(네이밍·상수화·추출·DRY), Before/After, 기준선 검증 |
| [05_단위테스트계획.md](./05_단위테스트계획.md) | 3단계 단위 테스트 계획(TC 24건, Red/Green, README 75~78 매핑) |
| [06_단위테스트구현.md](./06_단위테스트구현.md) | 3단계 단위 테스트 구현(26 TEST_F, ctest 25/26, TC_16 Red) |
| [07_결함분석.md](./07_결함분석.md) | ctest 실패 로그 결함 분석(DEF-001 P0, defect_list, 최소 수정안) |

상세 분석 원문:
- [docs/requirements_analysis.md](../docs/requirements_analysis.md)
- [docs/code_quality_report.md](../docs/code_quality_report.md)
- [docs/defect_list.md](../docs/defect_list.md)

2단계(1차 리팩토링) 완료 — [04_1차리팩토링.md](./04_1차리팩토링.md) 참고.  
3단계(단위 테스트) **구현** 완료(§9 0~4단계) — [06_단위테스트구현.md](./06_단위테스트구현.md), 계획 [05_단위테스트계획.md](./05_단위테스트계획.md), SSOT [docs/test_plan.md](../docs/test_plan.md).  
**결함 분석** 완료 — [07_결함분석.md](./07_결함분석.md), SSOT [docs/defect_list.md](../docs/defect_list.md). **TC_16 Green** 후 `ctest` 26/26 → 4단계(기능 개선). 5단계 회고 시 `01_실습보고서.md` 체크리스트·§3 정량 지표를 갱신하세요.
