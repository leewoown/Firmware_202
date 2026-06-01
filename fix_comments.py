# -*- coding: utf-8 -*-
# main.c 깨진 한글 주석 14줄 복원. 기존 들여쓰기/CRLF 보존, 깨진 텍스트만 교체.
import re

path = "SysSoure/main.c"

# 라인번호(1-indexed) -> 들여쓰기 뒤에 올 내용(주석 마커부터)
fixes = {
    126:  "*  인터럽트 함수 선언",
    195:  "*  인터럽트 함수 선언",
    364:  "* soc init 초기화하는 부분",
    459:  "// TODO: Balance 진입 조건 (원문 표현 확인 필요)",
    497:  "// 셀 전압 Balance",
    510:  "//SysRegs.HMICANErrCheck++; CPU 인터럽트 1msec",
    952:  "* 디지털 입력 상태 읽기   // TODO: 원문 표현 확인 필요",
    1525: "* PwrHoldCount 카운터 처리 (1초 주기 실행)",
    1527: "* 동작:",
    1528: "* - PWRRly(WakeUpOut) = 0 이면 카운트 증가",
    1529: "*   → 전원 출력 OFF 상태에서 Hold 유지 시간 누적",
    1531: "* - PWRRly(WakeUpOut) = 1 이면 카운터 초기화",
    1532: "*   → 전원 ON 상태에서는 Hold 타이머 의미 없음",
    1534: "* - 최대 14400초 (4시간)까지 카운트",
}

with open(path, "rb") as f:
    raw = f.read()

text = raw.decode("utf-8")          # FFFD 포함이지만 valid UTF-8 이라 OK
lines = text.split("\n")            # CRLF -> 각 줄 끝에 '\r' 보존됨

for ln, remainder in fixes.items():
    i = ln - 1
    old = lines[i]
    cr = "\r" if old.endswith("\r") else ""
    ws = re.match(r"[ \t]*", old).group(0)   # 기존 들여쓰기 그대로
    lines[i] = ws + remainder + cr

out = "\n".join(lines)
with open(path, "wb") as f:
    f.write(out.encode("utf-8"))        # UTF-8 / CRLF 유지

print("done")
