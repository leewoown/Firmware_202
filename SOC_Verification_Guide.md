# BMS SOC 알고리즘 검증 가이드

| 항목 | 내용 |
|---|---|
| 작성일 | 2026-05-05 |
| 작성자 | 이우원 |
| 검증자 | 김기현 선임 |
| 대상 펌웨어 | F28069PackBMS |
| 검증 범위 | SOC 초기화 / OCV-SOC 변환 |
| 셀 사양 | EVE LF230 (LFP, 230Ah) |
| 팩 구성 | 15S2P, 460Ah, DoD 80% (물리 SOC 10~90%) |

---

## 1. 검증 목적

이전 펌웨어에서 **만충 상태 부팅 시 SOC가 83% 로 표시되는 현상** 발생. OCV-SOC 변환 테이블이 사용 셀 spec과 불일치한 것이 원인. 신규 펌웨어 적용 후 다음을 확인.

1. 만충 상태 부팅 시 SOC 100% 정상 표시
2. 방전 종지 부근 SOC 0% 정상 표시
3. 평탄대 / 비평탄대 zone 분류 정확성
4. 신규 OCV-SOC 룩업 테이블 보간 정확성

---

## 2. 변경 사항 요약

| 파일 | 변경 내용 |
|---|---|
| BATAlgorithm.h | OCVPoint 구조체 / OCV_TABLE_SIZE / V_DispSoc0F / V_FlatStartF / V_FlatEndF / V_DispSoc100F 매크로 추가, 옛 매크로(V_MIN, V_Soc00~100, A1~B5, H_*) 주석 처리 |
| BATAlgorithm.c | 17점 LUT 테이블 정의 추가, CalEVE240AhSocInit 함수 LUT + 선형 보간 방식으로 교체, hermite_soc_40_60 함수 주석 처리 |
| DSP28x_Project.c | SysCalSocZoneHandle 함수의 zone 임계값을 신규 spec으로 교체 (3.295V / 3.340V / 3.360V) |

---

## 3. 사전 준비

### 3-1. 환경

| 항목 | 내용 |
|---|---|
| IDE | Code Composer Studio v12 |
| 프로젝트 | F28069PackBMS |
| 빌드 구성 | Debug |
| 디버거 | XDS100v2 또는 동등 JTAG 디버거 |
| 검증 대상 | 신규 빌드 산출물 `Debug/F28069PackBMS.out` |

### 3-2. 빌드 확인

```
1. CCS 실행 → 워크스페이스 → 프로젝트 열기
2. Project → Build Project (Ctrl + B)
3. Console 에서 "Build Finished" + 0 errors 확인
   ※ 옛 매크로/Hermite 함수가 주석 처리된 상태이므로 unused 경고 없음
```

### 3-3. 디버거 진입

```
1. F11 (Debug) → 타겟 다운로드 → 자동 정지
2. Window → Show View → Expressions → Expressions 뷰 열기
3. 다음 변수를 Add new expression 으로 등록
   - SysRegs.SysCellAgvVoltageF       (셀 평균 전압)
   - SysRegs.SysSocInitRule           (zone 판정 결과)
   - SysRegs.SysSOCF                  (최종 SOC 표시값)
   - EV240AhSocRegs.SysSocInitF       (OCV 산출 SOC)
   - EV240AhSocRegs.SysPackSOCF       (적산 산출 SOC)
   - EV240AhSocRegs.SoCStateRegs.bit.CalMeth   (SOC 산출 모드)
   - NVRZoneARDRegs.LastSOC           (NVR 저장 SOC, ×10)
```

---

## 4. 검증 절차

### 4-1. 디버거 강제 주입 방식 (셀 없이 빠르게 검증)

```
1. main.c 의 STANDBY 케이스 안쪽에 breakpoint 설정
   - 권장 위치: SlaveBMSIint 호출 직후, SysCalVoltageHandle 직전
   - 예: SysCalVoltageHandle(&SysRegs); 라인

2. 디버거 Resume (F8) → breakpoint 도달

3. Expressions 뷰에서 SysRegs.SysCellAgvVoltageF 값 우클릭 → Set Value
   각 검증 케이스의 입력 OCV 값 입력

4. Step Over (F6) 또는 Resume (F8) 으로 SysCalSocZoneHandle 통과
   → SysSocInitRule 값 결정됨 (Expressions 뷰에서 확인)

5. CalEVE240AhSocInit 함수 통과
   → EV240AhSocRegs.SysSocInitF 값 결정됨

6. STANDBY 끝까지 Resume → SysRegs.SysSOCF 최종값 확인

7. 다음 케이스 검증을 위해 BMS 리셋 후 재진입
```

### 4-2. 실제 셀 거치 방식 (현장 검증)

```
1. 모빌리티 충전 만충 상태로 만들기
2. BMS OFF → 잠시 대기 → BMS ON
3. STANDBY 통과 후 SOC 표시 확인
4. CAN 메시지 0x611 모니터링
   - 페이로드 4번째 워드(SysPackSOC) 값 확인
   - SysPackSOC = 1000 (= 100% × 10) 이면 정상
```

---

## 5. 검증 시나리오 — 5가지

### 케이스 1 — 만충 (가장 중요)

| 항목 | 값 |
|---|---|
| 입력 OCV | 3.360V (또는 그 이상) |
| 기대 zone | sharp high |
| 기대 SysSocInitRule | 1 (SOC_ZONE_cellVolt) |
| 기대 SysSocInitF | 100.00% |
| 기대 SysSOCF | 100.00% |
| 합격 기준 | SysSOCF = 100.0 ± 0.1% |

### 케이스 2 — 평탄대 중간

| 항목 | 값 |
|---|---|
| 입력 OCV | 3.300V |
| 기대 zone | flat |
| 기대 SysSocInitRule | 0 (SOC_ZONE_NVR) |
| 기대 SysSocInitF | NVR LastSOC / 10.0 |
| 합격 기준 | NVR 값과 일치 |

### 케이스 3 — 비평탄 저SOC

| 항목 | 값 |
|---|---|
| 입력 OCV | 3.250V |
| 기대 zone | sharp low |
| 기대 SysSocInitRule | 1 (SOC_ZONE_cellVolt) |
| 기대 SysSocInitF | 12.50% |
| 합격 기준 | 12.50 ± 0.5% |

### 케이스 4 — 방전 종지

| 항목 | 값 |
|---|---|
| 입력 OCV | 3.160V (또는 이하) |
| 기대 zone | sharp low |
| 기대 SysSocInitRule | 1 (SOC_ZONE_cellVolt) |
| 기대 SysSocInitF | 0.00% |
| 합격 기준 | 0.0 ± 0.1% |

### 케이스 5 — 평탄대 시작 경계

| 항목 | 값 |
|---|---|
| 입력 OCV | 3.295V |
| 기대 zone | flat |
| 기대 SysSocInitRule | 0 (SOC_ZONE_NVR) |
| 기대 SysSocInitF | NVR LastSOC / 10.0 |
| 합격 기준 | NVR 값과 일치 |

---

## 6. 신규 OCV-SOC 매핑 표 (참고용)

| 입력 OCV (V) | 출력 표시 SOC (%) | 물리 SOC (%) | 비고 |
|---|---|---|---|
| ≤ 3.160 | 0.00 | 10 | 방전 종지 (클램프) |
| 3.210 | 6.25 | 15 |  |
| 3.250 | 12.50 | 20 |  |
| 3.280 | 18.75 | 25 |  |
| 3.295 | 25.00 | 30 | 평탄대 시작 |
| 3.305 | 31.25 | 35 |  |
| 3.310 | 37.50 | 40 |  |
| 3.315 | 43.75 | 45 |  |
| 3.318 | 50.00 | 50 |  |
| 3.320 | 56.25 | 55 |  |
| 3.322 | 62.50 | 60 |  |
| 3.325 | 68.75 | 65 |  |
| 3.328 | 75.00 | 70 |  |
| 3.330 | 81.25 | 75 |  |
| 3.333 | 87.50 | 80 |  |
| 3.340 | 93.75 | 85 | 평탄대 끝 |
| ≥ 3.360 | 100.00 | 90 | 만충 (클램프) |

---

## 7. 결과 기록 양식

### 양식 (복사해서 사용)

```
==============================================================
BMS SOC 검증 결과 기록
==============================================================
검증자       : 김기현
검증 일시    : 2026-__-__  __:__
펌웨어 빌드  : F28069PackBMS Debug build
빌드 일자    : 2026-__-__
검증 방식    : [ ] 디버거 강제 주입    [ ] 실제 셀 거치
==============================================================

[케이스 1: 만충 OCV 3.360V] ★ 핵심 검증
  실제 SysCellAgvVoltageF :  3.____  V
  실제 SysSocInitRule     :  ____   (기대: 1)
  실제 SysSocInitF        :  ____.____ %  (기대: 100.00)
  실제 SysSOCF            :  ____.____ %  (기대: 100.00)
  판정: [ ] PASS    [ ] FAIL

[케이스 2: 평탄대 OCV 3.300V]
  실제 SysCellAgvVoltageF :  3.____  V
  실제 SysSocInitRule     :  ____   (기대: 0)
  실제 SysSocInitF        :  ____.____ %  (기대: NVR 값)
  실제 NVRZoneARDRegs.LastSOC : ____  (참고)
  판정: [ ] PASS    [ ] FAIL

[케이스 3: 비평탄 저SOC OCV 3.250V]
  실제 SysCellAgvVoltageF :  3.____  V
  실제 SysSocInitRule     :  ____   (기대: 1)
  실제 SysSocInitF        :  ____.____ %  (기대: 12.50)
  판정: [ ] PASS    [ ] FAIL

[케이스 4: 방전 종지 OCV 3.160V]
  실제 SysCellAgvVoltageF :  3.____  V
  실제 SysSocInitRule     :  ____   (기대: 1)
  실제 SysSocInitF        :  ____.____ %  (기대: 0.00)
  판정: [ ] PASS    [ ] FAIL

[케이스 5: 평탄대 경계 OCV 3.295V]
  실제 SysCellAgvVoltageF :  3.____  V
  실제 SysSocInitRule     :  ____   (기대: 0)
  실제 SysSocInitF        :  ____.____ %  (기대: NVR 값)
  판정: [ ] PASS    [ ] FAIL

==============================================================
종합 판정:  [ ] 전체 PASS    [ ] 일부 FAIL (재검토 필요)
==============================================================

특이 사항 / 메모:
  ___________________________________________________________
  ___________________________________________________________
  ___________________________________________________________

검증자 서명 : ________________________   일자: ____________
==============================================================
```

---

## 8. 합격 / 불합격 기준

### 합격 (PASS)

- 케이스 1 (만충) `SysSOCF = 100.0 ± 0.1%`
- 케이스 3, 4 보간 결과 `± 0.5%` 이내
- 케이스 2, 5 zone 분류 정확 (`SysSocInitRule` 값 일치)
- 모든 케이스에서 STANDBY 통과 후 안정 상태 유지

### 불합격 (FAIL)

- 케이스 1 만충값이 100% 가 아님 → 변경 코드 적용 누락 가능
- Zone 분류가 기대와 다름 → 임계값 적용 누락 가능
- STANDBY 단계에서 무한 루프 / 비정상 종료 → 코드 검토 필요

### 권장 추가 검증 (선택 사항)

- 8시간 운행 후 SOC 누적 오차 확인
- 충전 사이클 후 부팅 SOC 100% 자동 보정 확인
- NVR LastSOC 갱신 동작 확인 (3% 차이 시 저장)

---

## 9. 검증 환경 셋업 — 디버거 강제 주입 상세

### Set Value 사용 방법

```
1. Expressions 뷰에서 SysRegs.SysCellAgvVoltageF 우클릭
2. "Set Value..." 선택 (또는 단축키 Ctrl + W)
3. 입력 창에 값 입력 (예: 3.360)
4. OK 클릭
5. 변수 값이 변경되었는지 확인
```

### Breakpoint 위치 권장

```
파일: main.c
함수: main()
케이스: case STANDBY:
        ...
        SlaveBMSIint(&Slave3Regs);   ← 이 라인 직후에 breakpoint
        EV240AhSocRegs.state = SOC_STATE_INIT;
        ...
        SysCalVoltageHandle(&SysRegs);   ← 이 라인 직전에서 강제 주입
        ...
```

> SysCalVoltageHandle 호출 전에 SysCellAgvVoltageF 값을 강제 주입해야 후속 함수들이 그 값으로 동작.

---

## 10. 문제 발생 시 보고 형식

검증 중 예상과 다른 결과 발생 시 다음 정보를 모아서 보고.

```
[발생 케이스 번호]      : 케이스 ___
[입력 OCV]               : 3.____ V
[실제 측정 값들]
   - SysSocInitRule      : ____
   - SysSocInitF         : ____.____ %
   - SysSOCF             : ____.____ %
   - NVRZoneARDRegs.LastSOC : ____
[펌웨어 정보]
   - 빌드 일시            : ____
   - 변경 코드 적용 확인  : [ ] BATAlgorithm.h  [ ] BATAlgorithm.c  [ ] DSP28x_Project.c
[CCS Console 로그]
   - 빌드 시 경고/에러    : ____
[디버거 화면]
   - Expressions 뷰 캡처 (별첨)
[발생 빈도]
   - [ ] 항상 발생   [ ] 가끔 발생   [ ] 1회만 발생
```

---

## 11. 참고 자료

| 자료 | 위치 |
|---|---|
| EVE LF230 SOC-OCV 변환표 | (별도 첨부 이미지) |
| SOC 알고리즘 사양 정의 | 이 문서 §1, §2 |
| 변경 적용 라인 정보 | git diff 또는 변경 이력 참조 |

---

## 12. 진행 흐름 요약

```
[검증 시작]
   ↓
[빌드 통과 확인]
   ↓
[디버거 진입 + Expressions 등록]
   ↓
[케이스 1 (만충) 검증]  ← 핵심
   ↓
[케이스 2~5 순차 검증]
   ↓
[결과 기록 양식 작성]
   ↓
[종합 판정]
   ↓ PASS
[검증 완료 보고]
   ↓ FAIL
[원인 파악 → 재작업 → 재검증]
```

---

## 13. 연락처

검증 관련 문의 / 결과 보고:

- 담당자: 이우원
- 이메일: leewoowon95@outlook.com

---

문서 끝.
