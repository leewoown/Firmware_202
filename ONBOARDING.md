# F28069PackBMS — 온보딩 가이드

> 이 문서는 팀원이 Claude Code에서 이 펌웨어 프로젝트를 빠르게 파악하기 위한 온보딩 가이드입니다.

| 항목 | 내용 |
|---|---|
| 프로젝트 | F28069PackBMS (15S2P Pack BMS, Rev.02) |
| MCU | TI C2000 Piccolo **TMS320F28069** |
| IDE | Code Composer Studio **v12** (`C:\ti\ccs1271`) |
| 셀 | EVE **LF230** (LFP, 230Ah) |
| 팩 구성 | **15S2P**, 460Ah, DoD 80% (물리 SOC 10~90%) |
| Slave AFE | LTC6802 / LTC6804 계열 (SPI 데이지체인) |
| 통신 | CAN (eCAN-A), 마스터 TX `0x610~0x61F` / `0x701~0x705` |
| 저장소 | GitHub `leewoown/Firmware_45.git`, 기본 브랜치 `master` |

---

## 1. 프로젝트 개요

전기 모빌리티용 **15직렬 2병렬 LFP 배터리 팩 BMS** 펌웨어입니다.
TI F28069 단일 MCU가 마스터로 동작하며, LTC68xx Slave AFE로 셀 전압/온도를 읽고,
SOC 산출 · 셀 밸런싱 · 보호(Fault/Alarm/Protect) · 릴레이 시퀀스 · CAN 통신을 담당합니다.

핵심 기능 축:
- **SOC 산출** — OCV-SOC 룩업 + 쿨롱 카운팅 + zone 기반 모드 전환, NVRAM에 SOC 영속화
- **충전 제어** — CC→CV 전환 및 충전 종료 판정 (충전기 CAN 연동)
- **보호** — 과전압/저전압/과전류/과온/셀 편차 → Alarm/Protect/Fault 단계 처리
- **릴레이 시퀀스** — 메인 릴레이 ON/OFF, Pre-charge, 전원 홀드(PWRHOLD)

---

## 2. 코드베이스 구조

```
F28069PackBMS/
├─ SysSoure/              ← 애플리케이션 소스 (작업 대상)
│   ├─ main.c             ← 진입점·메인 루프 상태머신·ISR·SPI/CAN/LTC 드라이버·Slave BMS
│   ├─ DSP28x_Project.c   ← 시스템 계산 핸들러(전압/전류/온도/SOC zone)·cpu_timer0_isr
│   ├─ BATAlgorithm.c     ← SOC 알고리즘(CalEVE240Ah…)·OCV-SOC LUT·쿨롱 카운팅
│   ├─ BAT_LTC6802.c      ← Slave AFE(LTC68xx) 드라이버·PEC·셀 전압/밸런싱
│   ├─ ProtectRelay.c     ← 릴레이 시퀀스·보호 처리
│   └─ NVRAM.c            ← 비휘발성 SOC 저장/복원
├─ SysInclude/            ← 애플리케이션 헤더 (parameter.h, SysVariable.h, BATAlgorithm.h …)
├─ C2806XSrc/             ← TI F2806x 디바이스 지원 라이브러리 (수정 거의 안 함)
├─ C2806Xinclude/         ← TI 디바이스 헤더 + 프로젝트 공용 enum/struct(DSP28x_Project.h)
├─ Debug/                 ← 빌드 산출물 (.out/.map 등, 저장소에 포함됨)
├─ targetConfigs/         ← JTAG 타겟 설정(TMS320F28069.ccxml)
├─ .vscode/tasks.json     ← CCS 커맨드라인 빌드 태스크
└─ SOC_Verification_Guide.md  ← SOC 알고리즘 검증 절차서
```

> 작업은 거의 **`SysSoure/` + `SysInclude/`** 안에서 이뤄집니다. `C2806X*`는 TI 제공 코드라 손대지 않습니다.

핵심 전역 타입/enum은 [C2806Xinclude/DSP28x_Project.h](C2806Xinclude/DSP28x_Project.h)에 정의돼 있습니다 (`SystemReg`, `SysState`, SOC zone enum 등).

---

## 3. 빌드 / 디버그

### CCS IDE (디버깅 권장)
1. CCS 12 실행 → 워크스페이스 `D:\202 15S2P_PackBMSR02\.metadata`
2. `F28069PackBMS` 프로젝트 열기 → Build (Ctrl+B)
3. `F11`로 디버그 진입 → XDS100v2 등 JTAG로 타겟 다운로드

### VS Code 태스크 (커맨드라인 빌드)
[.vscode/tasks.json](.vscode/tasks.json)에 CCS 헤드리스 빌드가 등록돼 있습니다.
- **CCS Build (Debug)** — 기본 빌드 (Ctrl+Shift+B)
- **CCS Build (Release)** / **CCS Clean** / **CCS 열기**

산출물: `Debug/F28069PackBMS.out` (저장소에 함께 커밋됨).

---

## 4. 시스템 상태머신 (메인 루프)

[SysSoure/main.c](SysSoure/main.c)의 `main()` → `while(1)` → `switch(SysRegs.SysMachine)` 구조입니다.
상태 enum은 [DSP28x_Project.h:210](C2806Xinclude/DSP28x_Project.h#L210):

```
INIT(0) → STANDBY(1) → READY(2) → RUNING(3)
                              └→ PROTECTER(5)   (보호 진입)
                              └→ MANUALMode(6)  (AdminMode 시)
```

- **INIT** — 레지스터/타이머/CAN 초기화
- **STANDBY** — 셀 전압 읽기 → SOC zone 판정 → 초기 SOC 결정 (`INITOK` 셋)
- **READY** — 릴레이 ON 준비, PWRHOLD, 방전/충전 모드 결정
- **RUNING** — 정상 운전, 보호/통신 상태 갱신
- **PROTECTER** — 보호 트립 상태
- **MANUALMode** — 관리자/밸런싱 수동 제어

100ms 주기 처리는 `cpu_timer0_isr`(타이머0 인터럽트)에서 트리거됩니다.

---

## 5. SOC 알고리즘 (가장 중요)

### Zone 기반 모드 전환
부팅·운전 중 셀 평균 전압으로 **zone**을 판정해 SOC 산출 방식을 고릅니다.
([DSP28x_Project.h:225](C2806Xinclude/DSP28x_Project.h#L225))

| zone | 표시 SOC 구간 | 산출 방식 |
|---|---|---|
| `SOC_ZONE_NVR` | 평탄대 31.2~93.75% | NVRAM에 저장된 마지막 SOC 사용 |
| `SOC_ZONE_cellVolt` | 양 끝 0~31.2% & 93.75~100% | OCV-SOC LUT 보간 |

- 평탄대 경계: **3.305V(시작) / 3.340V(끝) / 3.360V(만충)**
- 경계 채터링 방지를 위해 셀 측정오차(±10mV) 기반 Hysteresis 적용
- 운전 중에는 **쿨롱 카운팅**으로 적산, zone 진입 시 OCV로 시드 재동기화 (점프 방지)

### OCV-SOC 룩업 테이블
17점 LUT + 선형 보간. 매핑표는 [SOC_Verification_Guide.md](SOC_Verification_Guide.md) §6 참조.
주요 함수: `CalEVE240AhRegsInit / CalEVE240AhSocInit / CalEVE240AhSocHandle` ([SysSoure/BATAlgorithm.c](SysSoure/BATAlgorithm.c))

### NVRAM
[SysSoure/NVRAM.c](SysSoure/NVRAM.c) — `LastSOC`(×10 저장) 등. SOC가 일정 폭 이상 바뀌면 저장.

> SOC 동작을 바꿀 때는 반드시 [SOC_Verification_Guide.md](SOC_Verification_Guide.md)의 5가지 케이스(만충/평탄대/저SOC/방전종지/경계)로 검증합니다.

---

## 6. 충전 제어 (CC / CV)

- SOC<90%: CC 구간, 팩 종단 전압 기준 + 마진
- 90%부터: CV 종단 전압으로 ramp, 충전 전류 임계 미만이면 CV 래치
- 셀 과전압 가드(약 3.55V) 적용
- 충전 종료: SOC 100% & 충전 전류 임계 이하
- 관련 처리: `PWRRlyHoldHandle` ([SysSoure/main.c](SysSoure/main.c)) + 충전기 CAN(`ChargerResgsStauts`)

---

## 7. 보호 / 릴레이

- `SysFaultCheck / SysAlarmtCheck / SysProtectCheck` ([SysSoure/main.c](SysSoure/main.c)) — 과/저전압, 과전류, 과온, 셀 편차 단계 판정
- `RlySeqHandle / ProtectRlyEMSHandle` ([SysSoure/ProtectRelay.c](SysSoure/ProtectRelay.c)) — 릴레이 시퀀스
- 보호 트립 시 `SysMachine = PROTECTER`로 전환

---

## 8. CAN 통신

- 마스터 → 외부: `0x610~0x61F`(BMS 상태/셀 전압/SOC 등), `0x701~0x705`
- 예: `0x611` 페이로드의 `SysPackSOC` = SOC×10 (100% → 1000)
- 수신: 충전기/VCU(PMS) 명령 (`ChargerResgsStauts`, `PMSCMDRegs`)
- TX: `CANATX(ID, len, d0, d1, d2, d3)` / RX: `ISR_CANRXINTA`

---

## 9. 빠른 시작 체크리스트

1. CCS 12 설치 확인 (`C:\ti\ccs1271`), F28069 JTAG 디버거 준비
2. 저장소 클론 후 `master` 브랜치 → CCS로 프로젝트 import / 빌드
3. 코드 진입점은 [SysSoure/main.c](SysSoure/main.c) `main()` — 상태머신부터 읽기
4. SOC 관련 작업이면 [SOC_Verification_Guide.md](SOC_Verification_Guide.md) 먼저 숙지
5. 전역 타입은 [DSP28x_Project.h](C2806Xinclude/DSP28x_Project.h)에서 확인

---

문서 끝.
