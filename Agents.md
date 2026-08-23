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
S0 — BOOT & INPUT
```

현재 목표는 게임 전체 구현이 아니다.

첫 번째 목표는 다음이다.

```text
SOURCE
→ BUILD
→ VALID .sfc ROM
→ PC EMULATOR BOOT
→ P1 PAD INPUT
→ P2 SNES MOUSE INPUT
```

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

# 7. Current Phase — S0 BOOT & INPUT

현재 Agent가 집중해야 할 범위는 **S0뿐**이다.

## S0 목적

다음 질문에 답한다.

> 이 저장소에서 실제 Super Famicom ROM을 안정적으로 반복 생성하고 입력을 읽을 수 있는가?

S0에서는 게임성을 평가하지 않는다.

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

---

# 11. Initial Toolchain Direction

현재 S0의 기본 기술 방향은 다음 계열을 우선한다.

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

S0에서는 이 입력을 게임 동작에 연결하지 않는다.

S0의 목적은 raw input 검증이다.

S1 이후에만 실제 조작으로 연결한다.

---

# 16. Future S1 Direction — Reference Only

S0가 공식적으로 완료된 뒤에만 S1을 시작한다.

S1의 예상 연구 대상은 다음과 같다.

```text
Player Tank
→ Hull Movement
→ Acceleration
→ Deceleration
→ Inertia
→ Limited Turning Rate
→ Hull Heading
```

그래픽은 placeholder를 허용한다.

S1의 핵심 질문은:

> SFC에서 RicochetAngles다운 전차 이동 자체가 성립하는가?

이다.

이 내용은 현재 작업 지시가 아니다.

---

# 17. Future Independent Turret Direction — Reference Only

S1 이후의 독립 포탑 검증에서 다음 구조를 우선 검토한다.

```text
Mouse ΔX / ΔY
→ Virtual Aim Position
→ Tank-to-Aim Direction
→ Turret Target Heading
```

차체와 포탑의 내부 heading은 독립적으로 유지한다.

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

현재 S0에서는 필요한 최소 검증만 구현한다.

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

현재 S0에서는 Map 작업을 하지 않는다.

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

세부 Boss 규칙을 현재 S0 코드에 미리 넣지 않는다.

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

현재 S0에서는 PCB, Flash ROM, EPROM, Shell을 설계하지 않는다.

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

6. P1 raw input

7. P2 Mouse raw input

8. Build / Verify

9. S0 review

10. GO / REVISE / DROP
```

현재 `1. Repository contract` 단계다.

---

# 45. Explicitly Forbidden Premature Work

현재 S0가 닫히기 전에 다음 작업을 선행하지 않는다.

```text
NO FINAL ART
NO 14W STAGE
NO BOSS IMPLEMENTATION
NO ARMOR SYSTEM
NO BULLET MODE
NO PHYSICAL CART
NO CUSTOM CONTROLLER
NO MUSIC PRODUCTION
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
S0 — BOOT & INPUT

CURRENT OBJECTIVE:
Generate and boot a valid SFC ROM,
then verify P1 Pad and P2 SNES Mouse raw input.

NEXT GATE:
S1 — DRIVE

DO NOT START S1
UNTIL S0 IS VERIFIED AND USER APPROVES GO.
```
