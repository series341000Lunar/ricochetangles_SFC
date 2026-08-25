# AGENTS.md

# RicochetAngles SFC Experimental — Agent Working Contract

이 문서는 `series341000Lunar/ricochetangles_SFC` 저장소에서 작업하는 AI Agent / Codex가 따라야 하는 **최상위 작업 계약**이다.

본 저장소는 RicochetAngles의 **Super Famicom / SNES Experimental Edition**을 위한 독립 저장소다.

이 프로젝트의 목적은 현대의 AI·에이전틱 코딩·자동화 기술을 활용하여 실제 Super Famicom 하드웨어에서 실행되는 RicochetAngles를 만들 수 있는지 단계적으로 검증하는 것이다.

본 저장소는 HTML Pilot 또는 Godot Edition의 포팅 작업공간이 아니다.

---

# 1. Repository Identity

## Repository

```text
series341000Lunar/ricochetangles_SFC
```

Visibility:

```text
PUBLIC
```

Role:

```text
RicochetAngles
└─ SFC Experimental
   └─ Native Super Famicom / SNES implementation
```

현재 개발 상태:

```text
S3 — CORE COMBAT
```

현재 목표는 게임 전체 구현이 아니다.

S0와 S1은 2026-08-23 사용자 GO 결정으로 `PASS / CLOSED`되었다. S2-02A는 사용자가 P1 SELECT Aim Mode 전환, P2 Pad 8방향 조준과 P2 B 발사를 직접 확인하여 2026-08-25 `PASS / USER CONFIRMED`됐다. 사용자의 GO 결정에 따라 S2도 `PASS / CLOSED`됐다. S3-01과 S3-01A-R1도 2026-08-25 사용자 직접 확인으로 `PASS / USER CONFIRMED`됐으며, 현재 목표는 S3-02R1의 Heavy Target silhouette와 front/rear/side 가독성 검증이다.

```text
PLAYER PROJECTILE
→ STATIC ENEMY AABB
→ HIT ONCE
→ HP -1
→ DESTROYED AT HP 0
```

Local Development Environment Contract

본 저장소의 주 개발 환경은 사용자의 Windows 개인 PC다.

현재 SFC Experimental은 이 환경을 Primary Development Machine으로 취급한다.

초기 S0 단계에서는 불필요한 환경 추상화보다 실제 주 개발기의 재현성과 안정성을 우선하므로 아래 경로를 하드코딩해 사용해도 된다.

Primary Windows User
Windows user:
LunarGagarin

사용자의 개인 Windows PC는 가능한 한 동일한 사용자 이름을 사용한다.

따라서 사용자 홈 경로가 필요한 경우 기본값은 다음으로 간주할 수 있다.

C:\Users\LunarGagarin

단 Git Repository 자체의 위치는 절대경로로 하드코딩하지 않는다.

Repository Root가 필요하면 현재 Git Working Tree에서 동적으로 확인한다.

예:

git rev-parse --show-toplevel
MSYS2

현재 설치된 MSYS2의 authoritative path:

C:\msys64

기본 개발 subsystem:

UCRT64

MSYS2 launcher:

C:\msys64\msys2_shell.cmd

관련 실행파일과 환경을 찾을 때 다른 MSYS2 설치본을 먼저 탐색하지 않는다.

S0에서 검증한 위 설치를 현재 S3에서도 기본 환경으로 사용한다.

위 경로가 존재하지 않거나 필요한 package가 없는 경우 임의로 다른 MSYS2/Cygwin/WSL 환경으로 우회하지 말고 현재 환경 상태와 필요한 package를 보고한다.

특히 다음 환경으로 자동 전환하지 않는다.

WSL
Cygwin
별도 Portable MSYS2
Git Bash 기반 대체 Toolchain

실제 필요성이 확인되고 사용자가 승인한 경우에만 개발환경 변경을 검토한다.

MesenCE

현재 주 SNES/SFC 개발 Emulator는 Mesen Community Edition이다.

설치 디렉터리:

C:\Users\LunarGagarin\Documents\MesenCE

실행파일:

C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe

S0에서 Emulator 실행 자동화가 필요한 경우 이 경로를 authoritative executable path로 사용한다.

예상 실행 대상:

C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe <ROM_PATH>

정확한 CLI argument 동작은 실제 S0-01에서 검증한 뒤 Build/Run script에 고정한다.

검증되지 않은 argument를 추측하여 계약으로 만들지 않는다.

MesenCE가 정상적으로 존재하는 한 다른 Emulator를 주 Emulator로 자동 변경하지 않는다.

Secondary Emulator

현재 Secondary SNES/SFC Emulator는 bsnes nightly다.

설치 디렉터리:

C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly

실행파일:

C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly\bsnes.exe

S0-04에서 동일 ROM의 Boot, P1 Pad, P2 SNES Mouse와 P1+P2 동시 입력을 사용자 확인했다. 정확한 nightly build number 또는 build date는 확인되지 않았으므로 추측하여 계약에 넣지 않는다.

Additional Compatibility Reference

Delta iOS는 추가 호환성 참고 환경이다. S0 ROM boot, 화면과 virtual P1 Pad는 `PASS / USER CONFIRMED`이며 P2 Mouse는 현재 환경에서 사용할 수 없어 `UNKNOWN`이다. Delta iOS는 실제 Super Famicom 검증이 아니다.

PVSnesLib / SFC Toolchain

S0-01에서 실제 빌드와 MesenCE 부팅에 성공한 canonical toolchain은 다음과 같다.

PVSnesLib version:

4.6.0

Windows install path:

C:\snesdev\pvsneslib-4.6.0

PVSNESLIB_HOME in MSYS2:

/c/snesdev/pvsneslib-4.6.0

Release archive:

pvsneslib_460_64b_windows_release.zip

SHA-256:

bfb651671af99cd0fdc64a469a3e9cbff53dee9c3e0b27879e15495e2de4f78e

Required MSYS2 build package:

make 4.4.1-3

저장소의 scripts/build.ps1은 위 경로를 process-local 환경으로 명시한다.
전역 PATH 또는 영구 PVSNESLIB_HOME 설정에 의존하지 않는다.

Toolchain 설치 위치를 매 Build마다 자동 검색하는 구조는 만들지 않는다.

한 번 S0에서 검증된 위치와 버전이 결정되면 그것을 본 프로젝트의 canonical development toolchain으로 사용한다.

Environment Modification Rule

Agent는 필요한 개발 도구가 없다고 판단될 경우 바로 임의 설치하지 말고 먼저 다음을 확인한다.

1. 이미 지정된 경로에 존재하는가?
2. 현재 설치된 버전은 무엇인가?
3. S0 목표에 실제로 필요한가?
4. 기존 환경을 변경하지 않고 사용할 수 있는가?

설치 또는 package 추가가 필요한 경우 변경 내용을 명확히 보고한다.

특히 기존 시스템 환경을 불필요하게 전역 변경하지 않는다.

가능하면 프로젝트별 Script와 명시적인 환경변수 설정을 사용한다.

Current Known Environment

현재 확인된 개발환경:

HOST
Windows PC

USER
LunarGagarin

REPOSITORY
series341000Lunar/ricochetangles_SFC
Repository location = resolve from current Git working tree

MSYS2
C:\msys64

MSYS2 SUBSYSTEM
UCRT64

PRIMARY EMULATOR
MesenCE

MESEN DIRECTORY
C:\Users\LunarGagarin\Documents\MesenCE

MESEN EXECUTABLE
C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe

SECONDARY EMULATOR
bsnes nightly

BSNES DIRECTORY
C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly

BSNES EXECUTABLE
C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly\bsnes.exe

ADDITIONAL COMPATIBILITY REFERENCE
Delta iOS

PVSNESLIB
Version 4.6.0

PVSNESLIB HOME
C:\snesdev\pvsneslib-4.6.0

PVSNESLIB HOME IN MSYS2
/c/snesdev/pvsneslib-4.6.0

COMPLETED GATE
S2 — INDEPENDENT TURRET — PASS / CLOSED / USER GO APPROVED 2026-08-25

CURRENT GATE
S3 — CORE COMBAT

CURRENT STATUS
S3-01 — PASS / USER CONFIRMED 2026-08-25
S3-01A-R1 — PASS / USER CONFIRMED 2026-08-25

CURRENT SUBTASK
S3-02R1 — Heavy Target Silhouette Revision

이 환경 정보가 실제 개발기와 다르다는 사실이 확인되면 조용히 다른 경로를 선택하지 않는다.

먼저 차이를 보고하고 이 계약을 갱신한다.


---

# 2. Project Relationship

RicochetAngles에는 서로 다른 세 개발 트랙이 존재한다.

```text
RicochetAngles
│
├─ HTML Pilot
│   └─ series341000Lunar/steel-angle-prototype
│
├─ Godot Edition
│   └─ series341000Lunar/ricochetangles
│
└─ SFC Experimental
    └─ series341000Lunar/ricochetangles_SFC
```

각 저장소는 별개의 Runtime과 개발 목적을 가진다.

## 절대 규칙

본 저장소 작업 중 다음 저장소를 수정하지 않는다.

```text
series341000Lunar/steel-angle-prototype
series341000Lunar/ricochetangles
```

HTML/Godot에서 필요한 설계나 데이터를 참고하는 것은 허용하지만 직접 dependency를 만들지 않는다.

필요한 데이터 공유가 생기는 경우 장기적으로:

```text
canonical source
→ exporter / converter
→ SFC-specific binary/data
```

방식을 우선한다.

HTML JavaScript 또는 Godot 코드를 SFC 코드에 직접 연결하지 않는다.

---

# 3. Source of Authority

구현 사실과 작업 판단의 우선순위는 다음과 같다.

```text
1. 현재 ricochetangles_SFC 저장소의 실제 코드와 빌드 결과
2. 이 AGENTS.md
3. 자동 검증 결과
4. SFC 전용 기술 계약 문서
5. RicochetAngles_SFC_Experimental_구상안 최신본
6. RicochetAngles 통합 프로젝트 기준서 최신본
7. BOSS_01 Combat Contract
8. 과거 문서와 과거 구현 이력
```

문서와 코드가 충돌하면 실제 현재 구현을 먼저 확인한다.

단 코드에 명백한 실수나 회귀가 있다고 판단되면 코드가 존재한다는 이유만으로 그것을 새로운 설계 계약으로 승격하지 않는다.

불명확하면 임의로 설계를 확정하지 말고 차이를 보고한다.

## Validation Authority and romdev Role

검증 권위는 다음 순서로 고정한다.

```text
scripts/build.ps1 + existing verifiers
→ authoritative build / static validation

romdev
→ headless automated runtime regression / memory debug

MesenCE
→ primary human emulator validation

bsnes
→ secondary emulator compatibility validation

Real Super Famicom
→ final hardware truth
```

romdev의 `PASS`는 MesenCE, bsnes 또는 실제 Super Famicom의 `PASS`를 의미하지 않는다. 자동 runtime regression, 사람의 조작·가독성 검증, cross-emulator compatibility와 real hardware 검증을 각각 분리해 보고한다.

romdev는 이 저장소에서 기존 `scripts/build.ps1` 산출물에 대한 Agent Regression / Debug Harness로만 사용한다.

* canonical PVSnesLib 4.6.0 / MSYS2 build 환경을 교체하지 않는다.
* romdev bundled compiler 또는 bundled toolchain으로 RicochetAngles를 재빌드하지 않는다.
* `scripts/build.ps1`이 생성한 `build\rom\*.sfc`만 romdev에 load한다.
* 기본 실행은 `HEADLESS`다.
* romdev `playtest`, SDL 또는 별도 GUI window는 사용자가 명시적으로 요청할 때만 연다.
* MesenCE 또는 bsnes를 Agent regression 과정에서 자동 실행하지 않는다.
* romdev 작업을 이유로 Git commit 또는 push를 실행하지 않는다.
* screenshot은 진단 artifact이며 gameplay memory assertion과 분리한다. screenshot mismatch만으로 gameplay hard `PASS` 또는 `FAIL`을 대체하지 않는다.

---

# 4. Experimental Status

본 프로젝트는 다음 상태다.

```text
EXPERIMENTAL
DROP-OK
NOT REQUIRED FOR MAIN PROJECT
```

완성을 전제로 개발하지 않는다.

각 단계에서 다음 중 하나를 판정한다.

```text
GO
REVISE
DROP
```

이미 투입한 시간이 많다는 이유만으로 다음 단계로 진행하지 않는다.

기술적으로 흥미롭더라도 RicochetAngles의 핵심 재미가 성립하지 않는다면 종료할 수 있다.

실패 역시 유효한 연구 결과다.

---

# 5. Core Design Identity

SFC판은 HTML판을 픽셀 단위로 복제하는 프로젝트가 아니다.

보존해야 할 것은 구현 방식이 아니라 **플레이어에게 요구하는 판단**이다.

RicochetAngles의 현재 핵심 정체성은 다음과 같다.

> 전장을 종횡무진하며 장갑각으로 방어하고, 정조준으로 적 전차의 기능을 해체하는 전차 헌팅 액션.

SFC판에서 우선 보존해야 할 핵심은 다음이다.

* 차체와 포탑의 방향이 독립적이다.
* 이동으로 거리와 사선을 만든다.
* 측면과 위치 선정이 전투 결과에 영향을 준다.
* 차체 방향이 장갑각과 방어 결과에 영향을 준다.
* 포탄 결과는 도탄 / 비관통 / 관통을 구분한다.
* 탄종 선택이 의미를 가진다.
* 약점 또는 기능 부위를 공격할 수 있다.
* 부위파괴가 전투 상황을 바꾼다.
* Boss Phase가 서로 다른 문제를 요구한다.
* 짧은 재도전과 하나의 완결된 Stage를 장기 목표로 한다.

---

# 6. Native SFC Implementation Rule

SFC판은 HTML 또는 Godot의 기계적 변환판이 아니다.

금지되는 사고방식:

```text
HTML JavaScript
→ 자동 변환
→ SFC
```

목표 구조:

```text
RicochetAngles Gameplay Rules
+
Validated Stage Intent
+
Shared Design Principles
        ↓
SFC-specific Reimplementation
        ↓
C / 65816 / PPU / binary assets
```

JavaScript 함수 구조나 Canvas object model을 그대로 재현하려 하지 않는다.

Godot Node 구조 역시 그대로 옮기지 않는다.

SFC 하드웨어에 적합한 구조로 다시 구현한다.

---

# 7. Completed Phase — S0 BOOT & INPUT

S0는 2026-08-23에 모든 최종 검증을 통과하고 사용자 GO 결정으로 `PASS / CLOSED`되었다.

## S0 목적

다음 질문에 답한다.

> 이 저장소에서 실제 Super Famicom ROM을 안정적으로 반복 생성하고 입력을 읽을 수 있는가?

S0에서는 게임성을 평가하지 않았다. 검증된 ROM build/input 구조는 이후 회귀검사의 `KNOWN GOOD S0 BASELINE`이다.

---

# 8. S0 Required Scope

S0에서 구현하거나 검증해야 하는 것은 다음이다.

## Required

* SFC Toolchain 설정
* 재현 가능한 ROM Build
* `.sfc` 파일 생성
* 정상적인 SNES/SFC ROM Header
* Emulator Boot
* 최소 화면 출력
* Build 식별 정보 표시 또는 확인
* P1 Standard Controller 입력 읽기
* P2 SNES Mouse 인식
* P2 Mouse 상대 이동 입력 읽기
* P2 Mouse Button 읽기
* Build 실패 시 명확한 오류 반환
* 최소 ROM sanity validation
* 반복 실행 가능한 build/verify 절차

가능하면 다음 흐름을 한 번의 명확한 명령 또는 wrapper로 실행할 수 있게 한다.

```text
build
→ verify
→ .sfc
```

---

# 9. S0 Out of Scope

S0에서는 다음 기능을 구현하지 않는다.

* 플레이어 전차
* 전차 이동 물리
* 가속
* 감속
* 관성
* 차체 선회
* 포탑
* 포탑 조준
* Projectile
* 주포
* 장갑
* 도탄
* 비관통
* 관통
* 탄종
* Enemy
* AI
* Stage
* Scroll Map
* Collision Map
* Boss
* Bullet Mode
* Focus
* Part Damage
* 최종 HUD
* Sprite Asset Pipeline
* 16방향 Sprite
* 32방향 Sprite
* 음악
* 최종 SFX
* Save System
* Flash Cartridge 대응 코드
* Physical Cartridge
* Dedicated Controller

S0에서 위 기능을 미리 만드는 것은 **scope violation**이다.

---

# 10. S0 Acceptance Gate

다음 조건을 모두 만족하면 S0를 완료한 것으로 판단할 수 있다.

```text
[1] 깨끗한 상태에서 ROM을 반복 빌드할 수 있다.
[2] 유효한 .sfc 파일이 생성된다.
[3] 주 개발 Emulator에서 정상 부팅된다.
[4] 별도의 Emulator에서도 기본 부팅을 교차 확인할 수 있다.
[5] P1 Pad 입력이 정상적으로 갱신된다.
[6] P2 SNES Mouse가 정상적으로 검출된다.
[7] Mouse 상대 이동값을 읽을 수 있다.
[8] Mouse Button 입력을 읽을 수 있다.
[9] Build/Verify 과정이 자동화 가능하다.
[10] HTML/Godot 저장소에 어떠한 수정도 필요하지 않다.
```

모두 통과하면:

```text
S0 CLOSED
→ GO S1
```

하나 이상의 핵심 항목이 구조적으로 해결되지 않는다면:

```text
REVISE
```

또는 필요하면:

```text
DROP
```

을 제안한다.

Agent가 독자적으로 다음 Gate로 넘어가지 않는다.

S0 최종 결과:

```text
STATUS: PASS / CLOSED
USER DECISION: GO
DATE: 2026-08-23
```

---

# 11. Initial Toolchain Direction

S0에서 확정한 기본 기술 방향은 현재 S3에서도 유지한다.

```text
C 중심 개발
+
필요한 경우 최소한의 65816 Assembly
+
Make 기반 ROM Build
```

현재 기본 SDK 방향:

```text
PVSnesLib
```

Windows 개발환경에서는 자동화 가능한 build 환경을 우선한다.

Emulator는 주 개발용과 최소 1개의 교차검증용 Emulator를 구분할 수 있다.

Toolchain의 정확한 버전은 저장소에서 실제로 검증된 버전을 기준으로 고정한다.

한 번 검증된 Toolchain 버전을 특별한 이유 없이 자동으로 최신 버전으로 올리지 않는다.

Toolchain 변경은 기능 추가와 동일하게 취급하고 변경 이유를 기록한다.

---

# 12. Initial ROM Contract

S0 초기 ROM은 최대한 단순한 구성을 사용한다.

초기 기본 방향:

```text
Mapping     : LoROM
Speed       : SlowROM
Type        : ROM only
SRAM        : None
Coprocessor : None
Enhancement : None
```

현재 실험 초기 단계에서는 Enhancement Chip을 전제로 하지 않는다.

다음 기능은 실제 필요성이 확인되기 전까지 도입하지 않는다.

* SuperFX
* SA-1
* DSP 계열
* Enhancement Chip 기반 우회
* 특수 Mapping
* 대형 SRAM 구조

하드웨어 한계가 발생했을 때 즉시 Enhancement Chip으로 우회하지 않는다.

먼저 해당 한계가 RicochetAngles의 핵심 체험을 실제로 막는지 판단한다.

---

# 13. Hardware Reality Rule

에뮬레이터에서 동작한다고 해서 실제 Super Famicom에서 동작한다고 가정하지 않는다.

장기적으로 반드시 다음 두 환경을 구분한다.

```text
EMULATOR VALIDATION
≠
REAL HARDWARE VALIDATION
```

실기 단계에서는 다음을 별도로 검증해야 한다.

* Boot stability
* Reset
* Controller
* SNES Mouse
* Input feel
* Frame pacing
* Sprite flicker
* Scanline sprite pressure
* DMA / VRAM pressure
* Actual display readability
* Audio timing
* 장시간 실행 안정성

Emulator의 정확도를 이용해 개발하되 실기 성공을 미리 선언하지 않는다.

---

# 14. Emulator Integrity Rule

성능 검증 시 실제 SFC보다 유리하게 만드는 Emulator 기능을 사용하지 않는다.

예:

* CPU overclock
* Enhancement chip overclock
* Sprite limit removal
* 비정상적인 latency reduction
* 기타 실제 하드웨어에 없는 성능 확장

디버깅 목적으로 이러한 기능을 임시 사용한 경우 반드시 명시한다.

성능 통과 판정은 실제 하드웨어에 가까운 설정에서만 한다.

---

# 15. Input Philosophy

RicochetAngles의 핵심 입력 철학은 다음이다.

```text
Hull
≠
Turret
```

초기 목표 입력 구조:

```text
P1 Standard SFC Pad
→ Hull

P2 SNES Mouse
→ Turret
```

S0에서는 이 입력을 게임 동작에 연결하지 않았다.

S0의 목적은 raw input 검증이다.

S1에서는 P1 Pad만 차체 이동에 연결한다. P2 Mouse의 실제 포탑 조작 연결은 후속 Gate 전까지 시작하지 않는다.

---

# 16. S1 Direction — Closed Gate Contract

`S1-01`, `S1-01R`, `S1-02`, `S1-02R`은 모두 2026-08-23 `PASS / USER CONFIRMED`됐다. 사용자는 최종적으로 32×32 Hull 가독성, CW/CCW, pivot, 전진/후진/복합 선회, 관성, P1/P2 회귀와 MesenCE/bsnes boot를 확인했고 S1에 GO를 승인했다. 따라서 S1은 `PASS / CLOSED / USER GO APPROVED 2026-08-23`이며 기존 이동 수치와 입력 구조는 S2의 Known Good Baseline이다.

S1의 초기 연구 범위는 다음과 같다.

```text
PLAYER HULL
→ D-PAD INPUT
→ DESIRED HEADING
→ LIMITED TURN RATE
→ THROTTLE
→ ACCELERATION
→ INERTIA
→ DECELERATION
→ POSITION UPDATE
```

그래픽은 placeholder를 허용한다.

S1의 핵심 질문은:

> SFC에서 RicochetAngles다운 전차 이동 자체가 성립하는가?

이다.

S1에서도 P2 Mouse gameplay, Turret, Aim Cursor, Fire, Projectile, Armor, Ricochet, Enemy, Stage, Boss, final art와 audio는 범위 밖이다.

---

# 17. Completed S2 / Current S3 Direction

S2-01의 다음 독립 포탑 구조는 `PASS / USER CONFIRMED 2026-08-23`다.

```text
Mouse ΔX / ΔY
→ Virtual Aim Position
→ Tank-to-Aim Direction
→ Turret Target Heading
```

차체와 포탑의 내부 heading은 독립적으로 유지한다.

S2-01은 Virtual Aim Cursor, integer vector-to-heading, limited traverse, 16-direction placeholder Turret와 compact diagnostic만 포함한다. P2 Mouse button은 raw diagnostic으로만 유지한다. Fire, Projectile, Reload, Recoil, Armor, Ricochet, Enemy, Map, Camera, Boss, final art와 audio는 범위 밖이다.

S2-01은 구현 직후 `IMPLEMENTED / USER PLAYTEST REQUIRED`였으며, 사용자가 Cursor, 독립 Turret, Hull 이동과 P1/P2 회귀가 모두 정상이라고 확인해 2026-08-23 `PASS / USER CONFIRMED`로 승격됐다.

현재 S2-02는 P2 SNES Mouse Left의 release 후 rising edge로만 발사하고, 발사 순간의 실제 `turret.heading`을 snapshot한 4-slot 정적 Projectile pool을 검증한다. 포탄은 Hull과 같은 Q8.8 위치 규칙, 기존 sin/cos LUT, 16 px muzzle offset, 4 px/frame 직선 속도와 18-frame 최소 cooldown을 사용한다. P2 Right는 raw diagnostic 전용이다.

S2-02는 `IMPLEMENTED / USER PLAYTEST REQUIRED`다. MesenCE와 bsnes boot, clean/deterministic build와 정적 계약 검사는 완료했지만, 독립 발사 방향, 선회/이동 중 발사, muzzle 정렬과 press-edge feel은 사용자 확인 전까지 PASS가 아니다. Enemy, target/wall collision, Armor, Ricochet, Damage, ammo/reload system, Focus, Map, Camera, Boss, final art와 audio는 범위 밖이다.

현재 S2-02A는 기존 Mouse 조준/발사를 그대로 보존하면서 P2 Standard Pad를 대체 조준 장치로 추가한다. 기본 Mode는 Mouse이며 P1 SELECT press edge로 Pad2와 전환한다. Pad2 D-pad는 축별로 독립 해석한 8방향 target heading과 Hull 기준 48 px indicator를 사용하고, P2 B는 Mouse Left와 같은 장치 독립 `fireHeld` 경로로 들어간다. Mode 전환은 발사를 disarm하고 release를 관찰하기 전까지 재발사를 막는다.

사용자는 P1 SELECT의 `AIM:M`/`AIM:P` 전환, `AIM:P`의 P2 D-pad 8방향 Turret Aim과 P2 B Main Gun Fire를 직접 확인했다. 따라서 S2-02A는 `PASS / USER CONFIRMED 2026-08-25`다. 이 확인과 사용자 GO 결정으로 S2는 `PASS / CLOSED / USER GO APPROVED 2026-08-25`다. P1+P2 동시 조작과 Aim Mode 전환 accidental-fire 방지는 이후 Gate의 지속 regression 항목으로 유지한다. 이 문장은 사용자가 확인하지 않은 세부 시나리오를 과거 시점의 `USER CONFIRMED`로 확장하지 않는다.

현재 S3-01은 정지한 Enemy Tank 1대에 대한 Player Projectile 충돌 foundation만 검증한다. Enemy는 위치, heading, HP 3, active와 짧은 hit blink 상태를 가지며 움직임, AI, 포탑, 발사 기능은 없다. 충돌은 Projectile 중심점과 회전하지 않는 단순 AABB로 처리하고, 유효한 hit는 shell을 즉시 비활성화한 뒤 damage 1을 정확히 한 번 적용한다. HP 0에서 Enemy OBJ를 숨기며 P1 SELECT는 DEV/diagnostic reset으로 Enemy와 Projectile pool을 초기화한다.

사용자는 2026-08-25 Enemy hit와 3회 유효 명중 뒤 destruction을 직접 확인했다. 따라서 S3-01은 `PASS / USER CONFIRMED 2026-08-25`다. 이 기록은 사용자가 이번 확인에서 별도로 열거하지 않은 Miss, Reset 또는 장치별 세부 시나리오까지 확인한 것으로 확장하지 않는다.

현재 S3-01A-R1은 P1 START Runtime Control Menu를 제공한다. 메뉴의 두 항목은 `DRIVE PC-LIKE/STICK`과 `AIM MOUSE/P2 PAD`이며 제목은 `RicochetAngles`, 하단 footer는 `CHANNEL A BNC`다. 기본값은 PC-LIKE와 MOUSE이고 선택값은 현재 실행 세션 동안 유지된다. STICK은 P1 D-pad를 8방향 world heading으로 해석해 기존 `TURN_RATE=2`와 가속/관성을 그대로 사용한다. P2 PAD Aim과 STICK Drive는 반대 축 동시 입력을 축별 neutral로 만드는 같은 작은 direction resolver를 공유한다.

메뉴가 열린 동안 Hull, Turret, Aim, Projectile, Enemy, hit flash와 fire cooldown gameplay update는 정지한다. START로 복귀할 때 fire input을 disarm하여 해당 장치의 발사 버튼 release 전 accidental shot을 막는다. START가 메뉴 전용이 됐으므로 S3-01 DEV Enemy Reset은 P1 SELECT로 이동했다. 기존 S0/S1 verbose raw diagnostic은 Bank 00 절감을 위해 compact HUD로 축소했지만 현재 Drive/Aim, Enemy HP, Hull/Turret heading, fire held와 active shell count는 남겼다.

S3-01A 초안은 Bank 00 free 28 bytes로 `oamInitGfxAttr`을 Bank 01로 밀어 final build가 실패했다. R1은 application-side 중복 방향 해석과 과거 verbose diagnostic formatting을 정리하여 Bank 00 free 5,301 bytes(16.18%)를 확보했고 `oamInitGfxAttr`을 Bank 00 `00:E922`에 복구했다. LoROM/SlowROM mapping, compiler option과 PVSnesLib 4.6.0은 변경하지 않았다. 사용자는 2026-08-25 START Menu, DRIVE PC-LIKE/STICK, AIM MOUSE/P2 PAD와 Arcade Cabinet 실제 플레이를 직접 확인했다. 따라서 S3-01A-R1은 `PASS / USER CONFIRMED 2026-08-25`다. Arcade Cabinet 결과는 `ADDITIONAL COMPATIBILITY / TWIN-STICK TEST — PASS / USER CONFIRMED`이며 실제 Super Famicom 검증이 아니다. SELECT는 cabinet의 정식 gameplay 경로로 가정하지 않는다.

현재 S3-02는 Enemy heading을 실제 gameplay armor geometry에 연결한다. Enemy는 local forward half-length 13 px, local right half-width 10 px의 회전된 rectangle과 FRONT/LEFT SIDE/RIGHT SIDE/REAR 4면을 가진다. Projectile previous/current integer-pixel segment를 Enemy-local 좌표로 변환하고, 두 boundary를 함께 통과한 corner에서는 정수 교차곱으로 더 이른 entry face 하나만 선택한다. Impact point는 integer world pixel, outward normal과 impact angle은 기존 0–255 heading 체계를 사용한다.

Impact angle 내부 표현은 `0..64`이며 0은 armor normal에 직각, 64는 surface를 스치는 90도 hit다. HTML 기준판의 일반 `autoRicochetAngleDegrees=75`를 보존한다. 0–255 heading 격자에서 75도 이상인 첫 단위는 54이므로 `impactAngle >= 54`를 RICOCHET으로 처리한다. 54 units는 75.9375도이며 HUD에는 정수 degree floor로 표시한다. RICOCHET은 shell을 종료하고 Enemy HP를 유지하며, 그 외 generic shell hit은 PENETRATION으로 기존 `damageEnemy` path를 통해 HP를 정확히 1 감소시킨다. NON-PENETRATION, armor thickness, penetration power와 reflected shell flight는 아직 구현하지 않는다.

S3-02 자동검증과 romdev headless runtime regression은 구현됐지만 MesenCE/bsnes의 사람이 보는 angle/readability와 실제 조작 체감은 별도 확인이 필요하다. 따라서 현재 상태는 `S3-02 — IMPLEMENTED / USER PLAYTEST REQUIRED`이며 사용자 승인 없이 S3-03으로 진행하지 않는다.

현재 S3-02R은 S3-02의 armor face/normal/impact-angle/RICOCHET/PENETRATION 의미와 75도(unit 54) threshold를 유지하면서 작은 HP3 dummy를 `HEAVY_TANK_TEST`로 교체한다. 최소 class 표현은 `EnemyState.classId` 하나이며 Heavy Test는 위치 `(223,144)`, heading 128, HP/max HP 20, AI/movement/turret/fire가 없는 정적 표적이다. PEN damage 1, RIC HP unchanged, 20 PEN destruction과 SELECT reset HP20을 사용한다. 과거 S3-01의 HP3 계약과 사용자 확인 기록은 당시 gate의 역사로 유지한다.

Heavy visual은 16방향 32×32 ochre module 두 개를 heading 앞/뒤 12 px에 합성한 약 54×30 px placeholder다. 전용 palette 3은 shadow/base/light/highlight의 ochre/sand 4단계이며 Player green과 분리된다. 전체 16방향 ROM graphics 8,192 bytes와 palette 32 bytes는 Bank 04에 두고, 현재 frame 512 bytes만 tile base 416의 네 VRAM row에 load하여 OBJ tile index 511을 넘지 않는다. Heavy OBJ는 OAM 48/52 두 개이고 전체 visible OBJ 최대 14개, worst gameplay scanline 26 OBJ tiles다.

Armor geometry는 visual에 맞춘 local half-length 28 px / half-width 16 px와 broad-phase radius 33 px다. Projectile visual/physics와 Ricochet math는 변경하지 않았다. 자동 verifier와 romdev headless WRAM regression은 HP20→19 PEN, HP19 유지 RIC, 20 PEN HP0/inactive를 확인했지만 Heavy scale/color/FRONT-SIDE readability/intentional Ricochet feel/Projectile 상대 크기는 사용자 확인 전까지 `UNVERIFIED`다. 따라서 상태는 `S3-02R — IMPLEMENTED / USER PLAYTEST REQUIRED`이며 S3-02를 PASS로 승격하거나 S3-03으로 진행하지 않는다.

현재 S3-02R1은 S3-02R gameplay 수치와 armor geometry를 그대로 유지하고 Heavy visual silhouette만 수정한다. 첫 revision의 tapered front/rear half가 가운데가 잘록한 skateboard처럼 읽힌다는 사용자 피드백에 따라, 16방향별 front/center/rear 직사각형 segment 세 개를 heading 축 `+17/0/-17 px`에 합성한다. 결과는 cardinal 기준 약 52×26 px의 box hull이며 pale front plate, dark upper/lower side armor band, dark flat rear plate를 얇게 구분한다. Ochre palette 3과 HP20, Projectile, Armor Face, impact-angle 및 unit 54 RICOCHET threshold는 변경하지 않는다.

S3-02R1 전체 ROM graphics는 front/center/rear 16방향 합계 24,576 bytes이고 palette는 기존과 같은 32 bytes다. 현재 frame set 1,536 bytes는 tile base 416/420/424의 분리된 row window에 load한다. Heavy OAM은 48/52/56 세 개이고 전체 visible OBJ 최대 15개, worst gameplay scanline 30/34 tiles다. 각 segment의 모든 frame은 32×32 edge에 닿지 않고 합성 centroid는 16방향 모두 동일하다. 상태는 `S3-02R1 — IMPLEMENTED / USER VISUAL REVIEW REQUIRED`이며 사용자에게 box-style Heavy Tank silhouette, front/rear, side readability와 현재 scale만 확인받는다. S3-02를 PASS로 승격하거나 S3-03으로 진행하지 않는다.

S3-02R1 box-hull 최종 clean build 2회는 동일한 262,144-byte ROM과 SHA-256 `42e6a96aca660d2aa033ae1e665fa45155f66f3a2d664cd92bcf51ad0eea3db9`를 생성했다. Compiler warning/error는 0이고 기존 S1/S2/S3/menu/armor verifier, Bank layout와 ROM_VERIFY가 모두 PASS했다. Bank 00은 1,240 bytes(3.78%) free이고 Heavy asset Bank 04는 8,160 bytes(24.90%) free다. romdev gameplay-preservation regression은 HP20 초기값, PEN 뒤 HP19, 20 PEN destruction과 threshold RIC HP19 유지를 확인했다. MesenCE/bsnes의 실제 silhouette 표시는 사용자 visual review 전까지 `UNVERIFIED`다.

실제 조작감이 좋지 않다면 입력 방식을 변경할 수 있다.

SNES Mouse를 사용한다는 이유만으로 나쁜 조작을 유지하지 않는다.

---

# 18. Graphics Direction — Future Contract

현재 비주얼 방향은 다음 계열을 유지한다.

* 완전 탑다운
* 극실사 배제
* Low-poly 3D pre-render
* 단색에 가까운 팔레트
* 낮은 Texture Density
* Posterized value
* Curvature 기반 edge highlight
* 강한 Hull/Turret silhouette
* 16bit palette에 적합한 색 분리
* 과도한 texture/noise 배제

하지만 S0에서는 최종 아트를 제작하지 않는다.

---

# 19. Directional Sprite Principle

향후 전차 방향 표현은 방향별 Sprite를 사용하는 것을 기본 후보로 한다.

초기 연구:

```text
Hull   : 16 directions
Turret : 16 directions
```

필요하면:

```text
Hull   : 32 directions
Turret : 32 directions
```

을 검토한다.

게임 내부 각도와 표시 Sprite 방향은 분리한다.

예:

```text
Internal Heading
0 ... 255

        ↓

Nearest Visual Frame
0 ... 15 / 31
```

장갑 판정을 표시 Sprite frame에 종속시키지 않는다.

단 실제 표시 방향과 판정 방향의 차이가 플레이어에게 불공정하면 해당 구조를 수정해야 한다.

---

# 20. Math / Simulation Principle

SFC에서 현대적인 부동소수점 물리 시뮬레이션을 그대로 재현하려 하지 않는다.

우선 검토할 수 있는 방식:

* Fixed-point
* Integer Vector
* Angle Lookup Table
* Sin/Cos Lookup Table
* Armor Face Direction Table

목표는 실제 탄도 시뮬레이션의 정확한 복제가 아니다.

목표는:

> 플레이어가 읽고 학습할 수 있는 RicochetAngles의 장갑각과 도탄 규칙

을 재현하는 것이다.

---

# 21. Asset Pipeline Philosophy

3D 모델은 SFC Runtime Asset이 아니다.

장기적으로 다음 역할을 가진다.

```text
3D Source Asset
→ Sprite Factory
```

예:

```text
Low-poly Tank
→ Direction Render
→ Crop / Pivot Normalize
→ Resize
→ Palette Quantize
→ SNES Graphics Conversion
→ Sprite / Tile Data
```

반복 가능한 작업은 가능한 한 자동화한다.

사람이 16방향 또는 32방향을 하나씩 수작업으로 처리하는 방식을 기본 파이프라인으로 만들지 않는다.

---

# 22. Generated Files Rule

Source와 Generated Artifact를 구분한다.

예상 구조:

```text
src/
assets/
tools/
scripts/

build/
generated/
```

원칙:

* 사람이 직접 편집하는 Source와 자동 생성 파일을 섞지 않는다.
* Generated file을 수정하여 문제를 해결하지 않는다.
* 재생성 가능한 Binary는 Source of Truth로 취급하지 않는다.
* 필요한 Generated file만 Git에 포함한다.
* 빌드 부산물은 가능한 한 `.gitignore` 한다.
* 최종 ROM을 버전 관리할지는 실제 배포 단계에서 별도로 결정한다.

---

# 23. Build Reproducibility

Agent가 새로운 빌드 단계를 추가하면 반드시 다음을 고려한다.

1. 어떤 도구가 필요한가.
2. 버전 의존성이 있는가.
3. Windows에서 반복 실행 가능한가.
4. 수동 GUI 조작 없이 실행 가능한가.
5. 실패했을 때 오류 원인이 드러나는가.
6. 새 환경에서 재구축 가능한가.

가능하면:

```text
one command
→ build
→ verify
```

를 유지한다.

---

# 24. Verification First

기능 구현만 하고 검증하지 않은 상태를 완료로 보고하지 않는다.

가능한 경우 각 작업에는 자동 검증을 추가한다.

향후 검증 후보:

* ROM 존재 여부
* ROM size sanity
* Header sanity
* Asset pointer bounds
* Tile count
* Palette count
* Sprite frame completeness
* Map converter consistency
* Collision bounds
* Lookup table generation
* Spawn/Event reference
* Boss state transition
* Build manifest
* Deterministic output check

S0에서는 필요한 최소 검증만 구현했다. 현재 S3에서도 현재 Gate에 필요한 검증만 추가한다.

전체 미래 검증 체계를 선제적으로 만들지 않는다.

---

# 25. Minimal Change Rule

한 작업에서 가능한 한 하나의 문제만 해결한다.

좋은 작업 단위:

```text
Toolchain bootstrap
→ verify

ROM boot
→ verify

P1 pad
→ verify

P2 mouse detection
→ verify
```

피해야 할 작업:

```text
Toolchain
+ Tank
+ Mouse turret
+ Projectile
+ Sprite converter
+ Stage
```

를 하나의 작업에서 동시에 만드는 것.

문제가 발생했을 때 원인을 분리할 수 있어야 한다.

---

# 26. No Premature Framework Rule

현재 프로젝트는 Experimental이다.

따라서 미래를 예상하여 범용 엔진을 먼저 만들지 않는다.

초기 단계에서 금지하는 예:

* 범용 ECS
* 범용 Event Graph
* 범용 Behavior Tree
* 범용 Scene Framework
* 범용 Script Language
* 대규모 Data Driven Framework
* Godot/HTML/SFC 공통 추상 엔진
* 필요성이 입증되지 않은 Asset Database
* 플러그인 시스템

현재 Gate가 요구하는 최소 구조를 먼저 만든다.

반복 비용이 실제로 확인된 이후에만 추상화를 도입한다.

---

# 27. No Premature Optimization Rule

65816과 SFC의 성능 제약을 무시하지 않는다.

하지만 처음부터 모든 코드를 Assembly로 작성하지 않는다.

우선순위:

```text
Correct
→ Measurable
→ Profile
→ Optimize
```

성능 문제가 실제로 확인되기 전에는 C 중심 구현을 유지한다.

Assembly는 다음과 같은 경우에만 검토한다.

* 실제 병목이 측정됨
* C 코드로 요구 프레임을 유지할 수 없음
* 해당 루틴이 명확히 국소화 가능
* Assembly 전환의 유지 비용이 합리적

Assembly 사용 자체를 기술적 성과로 취급하지 않는다.

---

# 28. Hardware Constraint Honesty

하드웨어 제한을 숨기지 않는다.

다음 문제가 발생하면 명확하게 보고한다.

* CPU budget 부족
* VRAM 부족
* DMA 부담
* Sprite flicker
* Scanline limit
* ROM size 증가
* Mouse polling 문제
* 입력 latency
* Palette 제약
* Asset conversion 수작업 증가
* Emulator / Real Hardware 차이

문제를 해결하기 위해 핵심 게임성을 조용히 제거하지 않는다.

다음 중 무엇인지 구분한다.

```text
IMPLEMENTATION BUG
TOOLCHAIN ISSUE
TUNING ISSUE
ASSET ISSUE
SFC HARDWARE LIMIT
DESIGN LIMIT
```

---

# 29. DROP Gate Philosophy

아래 문제가 구조적으로 확인되면 DROP 제안을 적극 허용한다.

* 차체/포탑 독립 조작이 충분히 성립하지 않는다.
* SNES Mouse가 실제 조준에 부적합하다.
* 화면이 너무 좁아 장갑각과 위협을 읽을 수 없다.
* Sprite/VRAM/CPU 제약이 핵심 전투를 심각하게 훼손한다.
* 방향 Sprite 관리 비용이 자동화로 해결되지 않는다.
* Toolchain 유지 비용이 연구 가치보다 크다.
* AI가 코드를 생성해도 디버깅에 과도한 수작업이 필요하다.
* 실기와 Emulator의 차이가 지속적으로 문제를 만든다.
* 핵심 재미를 유지하려면 SFC라는 플랫폼의 의미가 사라질 정도로 타협해야 한다.

DROP은 실패 보고가 아니다.

예:

```text
EXPERIMENTAL RESULT

TECHNICALLY INTERESTING
BUT NOT VIABLE

→ DROP
```

---

# 30. Main Project Protection Rule

SFC Experimental 때문에 다음을 변경하지 않는다.

* HTML Pilot의 H5E/H5F 로드맵
* HTML의 canonical gameplay rule
* Godot Edition 계획
* BOSS_01 본편 계약
* 기존 Map canonical data

SFC판에서 다른 설계가 더 잘 작동하더라도 자동으로 본편 규칙을 변경하지 않는다.

그 결과는 별도의 제안으로 보고한다.

---

# 31. 14W Stage Rule

현재 HTML Pilot의 Stage 구조는 장기 SFC Gold 목표의 참고 기준이다.

현재 상위 흐름:

```text
TUT
→ ADV
→ ENC
→ MBOSS / SAFE
→ ADV2
→ BOSS A/B/C
→ ENDING
→ RESULT
```

SFC판은 이 흐름의 **기능적 의미**를 보존하는 것이 목표다.

다음은 동일할 필요가 없다.

* Pixel 좌표
* 실제 Map 폭
* 적 개수
* Projectile 수
* HUD Layout
* Camera Margin
* VFX 밀도
* 정확한 Timing
* Boss Projectile Density

SFC Runtime이 HTML Map JSON을 직접 읽도록 만들지 않는다.

장기적으로 필요하면:

```text
HTML canonical Map/Data
→ Build-time Converter
→ SFC Binary
```

를 검토한다.

현재 S3-01에서도 Map 작업을 하지 않는다.

---

# 32. Boss Rule

BOSS_01의 실제 SFC 이식은 후반 작업이다.

현재 Boss 구조:

```text
A — Coupled Fortification
B — Mobile Heavy Core
C — Immobilized Last Stand
```

SFC에서도 세 Phase가 서로 다른 문제를 요구한다는 의미를 우선 보존한다.

세부 Boss 규칙을 현재 S3-01 코드에 미리 넣지 않는다.

BOSS_01 Combat Contract를 `AGENTS.md`에 복제하지 않는다.

실제 Boss 구현 단계에서 해당 최신 계약을 별도로 참조한다.

---

# 33. Dedicated Controller Rule

전용 컨트롤러는 초기 개발 범위가 아니다.

초기 조작:

```text
P1 Pad
+
P2 SNES Mouse
```

를 먼저 검증한다.

게임성이 확인된 뒤에만 다음을 검토한다.

```text
WASD-style P1 controller
Trackball
Dual D-pad
Custom combined enclosure
```

전용 컨트롤러를 만들기 위해 게임 입력 설계를 먼저 왜곡하지 않는다.

---

# 34. Physical Cartridge Rule

Physical Cartridge는 개발 도구가 아니다.

개발 순서:

```text
PC Emulator
→ Core Combat
→ Flash Cartridge
→ Real SFC
→ Full Stage
→ Physical Cartridge
```

Standalone Physical Cartridge 제작은 Stage가 실제 하드웨어에서 완주 가능한 수준에 도달한 이후에만 검토한다.

현재 S3-01에서는 PCB, Flash ROM, EPROM, Shell을 설계하지 않는다.

---

# 35. Documentation Rule

문서는 실제 개발에 필요한 만큼만 만든다.

현재 기본 문서:

```text
AGENTS.md
README.md
```

향후 실제 필요가 확인되면 다음과 같은 문서를 추가할 수 있다.

```text
SFC_TECH_CONTRACT_v0.1.md
SFC_INPUT_CONTRACT_v0.1.md
SFC_ASSET_PIPELINE_v0.1.md
```

하지만 S0 시작 전에 미래 계약 문서를 대량으로 만들지 않는다.

먼저 실제 ROM을 부팅한다.

확인된 사실을 기준으로 필요한 계약만 추가한다.

---

# 36. README Responsibility

`README.md`는 사람을 위한 프로젝트 소개와 기본 실행 안내를 담당한다.

`AGENTS.md`는 Agent의 작업 규칙을 담당한다.

둘의 역할을 혼합하지 않는다.

`README.md`에 이 문서 전체를 복제하지 않는다.

---

# 37. Commit Discipline

가능하면 작업은 작은 단위로 커밋할 수 있는 상태를 유지한다.

권장 단위:

```text
S0 toolchain bootstrap

S0 minimal ROM boot

S0 P1 pad input

S0 P2 mouse detection

S0 build verification
```

하나의 커밋에 무관한 대규모 변경을 섞지 않는다.

기능 구현과 대규모 정리 작업을 동시에 하지 않는다.

---

# 38. Refactor Rule

현재 동작하는 코드가 있다면 이유 없이 구조를 재작성하지 않는다.

Refactor는 다음 경우에만 한다.

* 명확한 버그 원인
* 중복이 실제로 반복됨
* 다음 Gate 진행을 방해함
* 성능 병목이 측정됨
* 유지보수 위험이 실제로 확인됨

“더 예뻐 보이는 구조”만을 이유로 대규모 재작성하지 않는다.

---

# 39. Error Handling

Toolchain 또는 Build에서 문제가 발생하면 우회하기 전에 원인을 분류한다.

예:

```text
ENVIRONMENT
PATH
TOOL VERSION
MAKEFILE
COMPILER
ASSEMBLER
LINKER
ROM HEADER
ASSET
EMULATOR
INPUT
```

오류를 숨기기 위해 실패 단계를 무조건 skip하지 않는다.

Fallback이 필요한 경우 원래 실패 이유와 fallback을 모두 기록한다.

---

# 40. Agent Reporting Format

작업 완료 후 Agent는 최소한 다음을 보고한다.

```text
작업 내용
변경 파일
빌드 결과
검증 결과
남은 문제
다음 Gate에 미치는 영향
```

가능하면 상태를 명시한다.

```text
PASS
PARTIAL
FAIL
BLOCKED
```

Gate 판단이 필요한 경우:

```text
GO
REVISE
DROP CANDIDATE
```

를 구분한다.

Agent가 사용자의 GO 결정을 대신하지 않는다.

---

# 41. No False Completion

다음 표현을 구분한다.

```text
IMPLEMENTED
TESTED
VERIFIED IN EMULATOR
VERIFIED ON REAL HARDWARE
USER CONFIRMED
```

예를 들어 Emulator에서 동작했다면:

```text
REAL HARDWARE VERIFIED
```

라고 보고하지 않는다.

자동 테스트만 통과했다면 조작감이 좋다고 선언하지 않는다.

---

# 42. User Role

사용자의 주요 역할은 다음이다.

* 프로젝트 방향 결정
* GO / REVISE / DROP 결정
* 실제 플레이
* 조작감 평가
* 그래픽 가독성 평가
* 실제 Super Famicom QA
* 필요 시 최소 3D Source Asset
* 향후 Controller Hardware 결정

사용자가 SFC PPU나 65816 Assembly를 숙달해야만 프로젝트가 진행되는 구조를 만들지 않는다.

AI와 자동화가 실무 구현의 가능한 많은 부분을 담당하도록 한다.

---

# 43. Agent Role

Agent / Codex가 가능한 한 담당할 영역:

* Toolchain
* Build Script
* C
* 필요한 최소 Assembly
* ROM 생성
* PPU / VRAM / DMA 처리
* Controller
* SNES Mouse
* Sprite 관리
* Projectile
* Armor calculation
* AI
* Asset converter
* Palette converter
* Map converter
* Lookup table generator
* ROM validation
* Regression
* Documentation

단 모든 기능을 한 번에 만들지 않는다.

현재 Gate가 요구하는 기능만 만든다.

---

# 44. Current Immediate Task Boundary

현재 작업 순서는 다음으로 제한한다.

```text
1. Repository contract
   → AGENTS.md

2. Minimal human-facing project information
   → README.md

3. S0 Toolchain bootstrap

4. Minimal ROM

5. Emulator boot

6. P1 raw input — VERIFIED 2026-08-23

7. P2 Mouse raw input — VERIFIED 2026-08-23

8. Build / Verify — VERIFIED 2026-08-23

9. S0 final regression / cross-emulator review — VERIFIED 2026-08-23

10. GO — USER APPROVED 2026-08-23

11. S1 — DRIVE — PASS / CLOSED / USER GO APPROVED 2026-08-23

12. S1-01 — Hull Movement V0 — PASS / USER CONFIRMED 2026-08-23

13. S1-01R — Tank Control Revision — PASS / USER CONFIRMED 2026-08-23

14. S1-02 — 16-Direction Hull Presentation V0 — PASS / USER CONFIRMED 2026-08-23

15. S1-02R — Hull Scale & Readability Revision — PASS / USER CONFIRMED 2026-08-23

16. S2 — INDEPENDENT TURRET — PASS / CLOSED / USER GO APPROVED 2026-08-25

17. S2-01 — Virtual Aim & Independent Turret V0 — PASS / USER CONFIRMED 2026-08-23

18. S2-02 — Main Gun Fire & Projectile V0 — IMPLEMENTED

19. S2-02A — Alternate Aim Input / P2 Pad V0 — PASS / USER CONFIRMED 2026-08-25

20. S3 — CORE COMBAT — ACTIVE

21. S3-01 — Static Enemy Target & Projectile Hit V0 — PASS / USER CONFIRMED 2026-08-25

22. S3-01A-R1 — Bank 00 Size Reduction / Runtime Control Menu Salvage — PASS / USER CONFIRMED 2026-08-25

23. S3-02 — Armor Face / Impact Angle / Ricochet V0 — IMPLEMENTED / USER PLAYTEST REQUIRED

24. S3-02R — Heavy Armor Target / Ricochet Readability Revision — IMPLEMENTED / USER PLAYTEST REQUIRED

25. S3-02R1 — Heavy Target Silhouette Revision — IMPLEMENTED / USER VISUAL REVIEW REQUIRED
```

S0는 MesenCE 2.2.1과 bsnes nightly의 사용자 확인, Delta iOS 추가 호환성 확인, 결정적 clean build와 ROM sanity를 근거로 `PASS / CLOSED`되었다.

S0 종료 시점의 ROM/build/input 구현은 `KNOWN GOOD S0 BASELINE`이다. 이후 Gate에서 Boot 또는 Input 회귀가 발생하면 이 상태와 비교하며, 새 구현을 이유로 정상 동작하는 S0 build/input 구조를 불필요하게 재작성하지 않는다.

실제 Super Famicom은 계속 `UNVERIFIED`이며 향후 Real Hardware Gate에서 별도로 검증한다.

---

# 45. Explicitly Forbidden Premature Work

현재 S3-02 범위에서는 다음 작업을 선행하지 않는다.

```text
NO ENEMY AI OR MOVEMENT
NO ENEMY TURRET OR FIRE
NO TANK-vs-TANK COLLISION
NO RELOAD SYSTEM BEYOND THE MINIMAL COOLDOWN
NO RECOIL
NO NON-PENETRATION
NO ARMOR THICKNESS OR PENETRATION POWER
NO REFLECTED PROJECTILE FLIGHT
NO AMMO TYPE
NO PLAYER HP
NO MAP
NO CAMERA
NO STAGE
NO BOSS
NO FINAL ART
NO 32-DIRECTION UPGRADE
NO AUDIO
NO PHYSICAL CART
NO CUSTOM CONTROLLER
NO GENERAL ENGINE
NO GODOT SHARED CODE
NO HTML SHARED RUNTIME
```

---

# 46. Design Change Rule

구현 중 기존 설계와 충돌하는 문제가 발견되면 다음 순서로 처리한다.

```text
1. 실제 문제를 재현한다.
2. 구현 버그인지 하드웨어 한계인지 구분한다.
3. 최소 수정으로 해결 가능한지 확인한다.
4. 해결 불가하면 SFC 전용 변경안을 제안한다.
5. 사용자 판단을 기다린다.
```

본편의 설계를 자동으로 수정하지 않는다.

---

# 47. Scope Expansion Rule

새로운 시스템을 추가하기 전에 다음 질문에 답한다.

1. 현재 Gate를 통과하는 데 필요한가?
2. 현재 문제를 더 작은 수정으로 해결할 수 없는가?
3. 실제 반복 비용이 확인됐는가?
4. 이 시스템이 SFC Experimental의 연구 목적에 직접 기여하는가?
5. 삭제하더라도 핵심 테스트가 가능한가?

대부분의 답이 `NO`라면 지금 만들지 않는다.

---

# 48. Final Working Principle

이 저장소에서 가장 중요한 개발 순서는 다음이다.

```text
BOOT
→ INPUT
→ MOVEMENT
→ TURRET
→ FIRE
→ RICOCHET
```

그 뒤에야:

```text
AMMO
→ FOCUS
→ PART DAMAGE
→ ENEMY
→ STAGE
→ BOSS
→ ART
→ AUDIO
→ REAL HARDWARE POLISH
→ PHYSICAL CARTRIDGE
→ DEDICATED CONTROLLER
```

를 검토한다.

현재 가장 중요한 문장은 다음이다.

> **먼저 실제 ROM을 띄운다.**

그 다음:

> **움직여본다.**

그 다음:

> **포탑을 따로 돌려본다.**

그 다음:

> **도탄시켜본다.**

각 단계에서 실제 결과를 확인한 뒤에만 다음 단계로 진행한다.

---

# 49. Current Status

```text
PROJECT:
RicochetAngles SFC Experimental

REPOSITORY:
series341000Lunar/ricochetangles_SFC

STATUS:
EXPERIMENTAL / DROP-OK

CURRENT GATE:
S3 — CORE COMBAT

CURRENT OBJECTIVE:
Verify that Enemy heading changes the impacted armor face and that
the same generic shell penetrates or ricochets by impact angle.

COMPLETED GATE:
S2 — INDEPENDENT TURRET — PASS / CLOSED / USER GO APPROVED 2026-08-25

CURRENT STATUS:
S3-01 — PASS / USER CONFIRMED 2026-08-25
S3-01A-R1 — PASS / USER CONFIRMED 2026-08-25
S3-02 — IMPLEMENTED / USER PLAYTEST REQUIRED
S3-02R — IMPLEMENTED / USER PLAYTEST REQUIRED
S3-02R1 — IMPLEMENTED / USER VISUAL REVIEW REQUIRED

CURRENT SUBTASK:
S3-02R1 — Heavy Target Silhouette Revision

DO NOT MARK S3-02 PASS OR PROCEED TO S3-03, NON-PEN,
ARMOR THICKNESS OR PENETRATION POWER WITHOUT USER CONFIRMATION.
```
