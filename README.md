# RicochetAngles — SFC Experimental

**현대의 생성형 AI·에이전틱 코딩·자동화 기술을 활용하여 실제 Super Famicom / SNES 하드웨어에서 동작하는 RicochetAngles 신작을 만들 수 있는지 검증하는 독립 Experimental 프로젝트입니다.**

> **현재 상태:** `S3 — CORE COMBAT / S3-01 IMPLEMENTED / USER PLAYTEST REQUIRED`
> **완료 Gate:** `S2 — INDEPENDENT TURRET / PASS / CLOSED / USER GO APPROVED 2026-08-25`
> **Project Status:** `EXPERIMENTAL / DROP-OK`
> **Target Hardware:** Super Famicom / SNES
> **Repository:** `series341000Lunar/ricochetangles_SFC`

---

## 프로젝트 개요

RicochetAngles는 다음과 같은 핵심 플레이를 가진 탑다운 전차 헌팅 액션입니다.

> **전장을 종횡무진하며 장갑각으로 방어하고, 정조준으로 적 전차의 기능을 해체하는 전차 헌팅 액션.**

본 저장소는 RicochetAngles의 **Super Famicom / SNES 전용 네이티브 구현 가능성을 검증하는 독립 개발 트랙**입니다.

목표는 단순히 SFC 스타일의 그래픽을 만드는 것이 아닙니다.

실제로 가능한 경우 다음 단계까지 진행합니다.

```text
Source Code
→ SFC ROM (.sfc)
→ PC Emulator
→ Flash Cartridge
→ Real Super Famicom
→ Complete SFC Stage
→ Physical Cartridge
```

다만 이 프로젝트는 반드시 완성해야 하는 프로젝트가 아닙니다.

SFC 하드웨어의 구조적 한계, 입력 방식, 개발 비용 또는 디버깅 비용 때문에 RicochetAngles의 핵심 체험이 성립하지 않는다고 판단되면 해당 Gate에서 종료할 수 있습니다.

실패 역시 유효한 기술검증 결과로 취급합니다.

---

## RicochetAngles 프로젝트 내 위치

RicochetAngles는 현재 세 개의 독립 Runtime 트랙으로 나뉩니다.

```text
RicochetAngles
│
├─ HTML Pilot
│   └─ steel-angle-prototype
│
├─ Godot Edition
│   └─ ricochetangles
│
└─ SFC Experimental
    └─ ricochetangles_SFC
```

각 저장소의 역할은 분리되어 있습니다.

| Track                | Repository                                 | 역할                               |
| -------------------- | ------------------------------------------ | -------------------------------- |
| HTML Pilot           | `series341000Lunar/steel-angle-prototype`  | 현재 게임 규칙과 Stage의 기준 구현           |
| Godot Edition        | `series341000Lunar/ricochetangles`         | 향후 Engine Edition                |
| **SFC Experimental** | **`series341000Lunar/ricochetangles_SFC`** | **Super Famicom 네이티브 구현 및 기술검증** |

SFC판은 HTML JavaScript나 Godot 코드를 자동 변환하는 포팅 프로젝트가 아닙니다.

```text
RicochetAngles Gameplay Rules
+
Validated Stage / Combat Intent
        ↓
SFC-specific Reimplementation
        ↓
C / 65816 / SNES Hardware
```

게임 규칙과 설계 의도는 공유하지만 Runtime은 독립적으로 구현합니다.

---

# 현재 개발 단계

## S3 — CORE COMBAT

S0 — BOOT & INPUT과 S1 — DRIVE는 2026-08-23 사용자 GO 결정으로 각각 `PASS / CLOSED`됐습니다. 사용자는 2026-08-25 S2-02A의 Aim Mode 전환, P2 Pad 8방향 Turret Aim과 P2 B Main Gun Fire를 직접 확인했고 S2 진행 결과에 GO를 승인했습니다. 따라서 S2도 `PASS / CLOSED`됐습니다.

현재 작업:

```text
S3-01 — Static Enemy Target & Projectile Hit V0
IMPLEMENTED / USER PLAYTEST REQUIRED
```

현재 S3-01 검증 범위:

```text
PLAYER PROJECTILE
→ STATIC ENEMY AABB
→ HIT ONCE
→ HP -1
→ DESTROYED AT HP 0
```

### S3-01에서 하지 않는 것

이번 작업은 정적 Enemy에 대한 Projectile collision foundation만 구현합니다.

* Enemy AI / 이동 / Turret / Fire
* Player HP / Tank-vs-Tank collision
* Armor Face / Impact Angle
* Ricochet / Non-Penetration / Penetration
* Ammo Type / Reload / Recoil / Explosion
* Map / Camera / Stage / Boss
* Final art / Audio
* Real hardware / Physical Cartridge / Custom Controller

---

# 개발 Gate

개발은 기능을 한 번에 확장하지 않고 Gate 단위로 진행합니다.

```text
S0 — BOOT & INPUT
↓
S1 — DRIVE
↓
S2 — INDEPENDENT TURRET
↓
S3 — CORE COMBAT
↓
S4 — REAL HARDWARE
↓
Stage Adaptation
↓
Physical Cartridge / Dedicated Controller
```

각 단계가 끝날 때 다음 중 하나를 판단합니다.

```text
GO
REVISE
DROP
```

---

## S0 — BOOT & INPUT — PASS / CLOSED

PC Emulator에서 정상적인 SFC ROM을 반복 빌드하고 기본 입력을 읽습니다.

핵심 질문:

> 우리가 안정적으로 SFC 프로그램을 만들 수 있는가?

---

## S1 — DRIVE — PASS / CLOSED

플레이어 전차의 차체 이동을 구현합니다.

검증 대상:

* 가속
* 감속
* 관성
* 제한 선회
* 차체 방향

핵심 질문:

> SFC에서도 RicochetAngles다운 전차 이동이 성립하는가?

---

## S2 — INDEPENDENT TURRET — PASS / CLOSED

초기 입력 후보:

```text
P1 Standard SFC Pad
→ Hull

P2 SNES Mouse
→ Turret
```

차체와 포탑의 방향을 완전히 분리합니다.

핵심 질문:

> SFC 입력 환경에서 차체와 포탑을 독립적으로 조작하는 것이 충분히 재미있고 정밀한가?

---

## S3 — CORE COMBAT

최소 전투 구조를 구현합니다.

```text
Player Tank
+
Enemy Tank
+
Projectile
+
Armor Face
+
Impact Angle
+
Ricochet
+
Non-Penetration
+
Penetration
```

핵심 검증 루프:

```text
이동
→ 사선 형성
→ 포탑 조준
→ 발사
→ 장갑각에 따라 결과 변화
```

이 단계까지 성공하면 최소 연구 목표는 달성한 것으로 볼 수 있습니다.

---

## S4 — REAL HARDWARE

Core Combat가 충분히 성립한 뒤에만 실제 Super Famicom에서 검증합니다.

```text
SFC ROM
→ SD / Flash Cartridge
→ Vanilla Super Famicom
```

검증 대상:

* Boot stability
* 실제 Controller / Mouse 입력
* Frame pacing
* Sprite flicker
* Scanline 제한
* VRAM / DMA 부담
* 실제 화면에서의 가독성
* Audio timing
* 장시간 실행 안정성

에뮬레이터 성공을 실기 성공으로 간주하지 않습니다.

---

# 성공 단계

## Bronze

PC Emulator에서 다음이 동작합니다.

* ROM Boot
* Tank Movement
* Independent Turret
* Fire
* Basic Armor / Ricochet

최소 연구 성공 단계입니다.

## Silver

실제 Super Famicom + Flash Cartridge에서 핵심 전투가 정상 실행됩니다.

## Gold

현행 RicochetAngles HTML Pilot의 핵심 Stage 구조에 대응하는 SFC Stage를 실제 하드웨어에서 완주할 수 있습니다.

## Master

* 최종 SFC ROM
* 독립 Physical Cartridge
* 확정 입력장치 또는 Dedicated Controller
* 실제 Super Famicom에서 완결 플레이

---

# 핵심 게임 원칙

SFC판이 RicochetAngles로 성립하려면 다음 요소를 우선 보존합니다.

* 차체와 포탑의 독립 방향
* 이동을 통한 사선 형성
* 측면 기동
* 장갑각
* 도탄 / 비관통 / 관통
* 탄종 선택
* 정조준 또는 이에 대응하는 공격 판단
* 기능 부위 또는 약점 공격
* 부위파괴가 전투 상황에 미치는 영향
* 서로 다른 문제를 요구하는 Boss Phase

반대로 다음 요소는 SFC 하드웨어에 맞게 변경할 수 있습니다.

* 정확한 Map 크기
* 적 수량
* Projectile 수
* HUD Layout
* Camera Margin
* VFX
* Alpha 표현
* Boss 공격 밀도
* Timing
* Stage 길이

**픽셀 동일성보다 플레이어가 수행하는 판단을 보존하는 것을 우선합니다.**

---

# 그래픽 방향

SFC판은 Low-poly 3D를 Runtime에서 사용하는 게임이 아닙니다.

3D Asset은 방향별 Sprite를 만드는 **Sprite Factory Source**로 사용합니다.

예상 파이프라인:

```text
Low-poly 3D Tank
↓
Directional Render
↓
Resize / Crop
↓
Palette Quantization
↓
SNES 4bpp Conversion
↓
Sprite / Tile Data
```

현재 기본 비주얼 방향:

* Top-down
* Low-poly pre-render
* 낮은 Texture Density
* 단색 계열
* Posterized Value
* 밝은 Edge Highlight
* 명확한 Hull / Turret Silhouette
* 제한된 Palette
* 과도한 Noise 배제

초기에는 16방향 Sprite를 우선 검증하고, 실제 조작감 개선 효과가 있을 경우 32방향을 검토합니다.

---

# 입력 방향

초기 기준 입력:

```text
P1
Standard SFC Controller
→ Hull

P2
SNES Mouse
→ Turret
```

장기적으로 게임성이 확인되면 다음과 같은 전용 입력장치를 검토할 수 있습니다.

* WASD-style Controller
* D-pad + Trackball
* Dual D-pad
* Mouse / Trackball derivative
* P1/P2 입력을 하나의 Enclosure로 통합한 Dedicated Controller

전용 Controller는 게임성이 먼저 확인된 뒤에 제작합니다.

---

# 기술 방향

초기 개발은 다음 방향을 우선 검토합니다.

```text
C
+
필요한 최소 65816 Assembly
+
Make-based Build
```

S0-01에서 고정한 SDK:

```text
PVSnesLib 4.6.0
C:\snesdev\pvsneslib-4.6.0
```

초기 ROM은 가능한 한 단순한 구성을 사용합니다.

```text
Mapping     : LoROM
Speed       : SlowROM
Type        : ROM only
SRAM        : None
Coprocessor : None
Enhancement : None
```

실제 검증을 통해 필요성이 확인되기 전에는 Enhancement Chip을 전제로 설계하지 않습니다.

이 구성으로 Windows/MSYS2 빌드와 MesenCE 부팅을 실제 확인했습니다.

---

# Build

## S0-01 검증 환경

2026-08-23에 다음 구성을 실제 사용했습니다.

| 구성 요소 | 버전 / 경로 |
| --- | --- |
| MSYS2 | `C:\msys64` / runtime `3.6.9-2` / UCRT64 |
| GNU Make | `4.4.1-3` |
| PVSnesLib | `4.6.0` |
| PVSnesLib 설치 경로 | `C:\snesdev\pvsneslib-4.6.0` |
| 816-tcc | `0.9.25` |
| WLA-65816 | `10.7a` |
| WLALINK | `5.22a` |
| MesenCE | `2.2.1` |
| MesenCE 실행 파일 | `C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe` |

사용한 PVSnesLib 공식 Windows 릴리스 파일은 `pvsneslib_460_64b_windows_release.zip`이며 SHA-256은 다음과 같습니다.

```text
bfb651671af99cd0fdc64a469a3e9cbff53dee9c3e0b27879e15495e2de4f78e
```

저장소 스크립트는 `PVSNESLIB_HOME=/c/snesdev/pvsneslib-4.6.0`과 UCRT64 환경을 해당 프로세스에만 설정합니다. 전역 PATH나 영구 사용자 환경변수 변경은 필요하지 않습니다.

## Build + Verify

저장소 루트의 PowerShell에서 실행합니다.

```powershell
.\scripts\build.ps1
```

이 명령은 기존 `build/`만 안전하게 비운 뒤 별도 작업 디렉터리에서 clean build하고, ROM sanity check까지 수행합니다.

성공 시 최종 ROM은 다음 위치에 생성됩니다.

```text
build\rom\ricochetangles_s0_hello.sfc
```

ROM 검증만 다시 실행하려면 다음 명령을 사용합니다.

```powershell
.\tools\verify-rom.ps1 -RomPath .\build\rom\ricochetangles_s0_hello.sfc
```

검증 항목:

* 256 KiB power-of-two ROM 및 32 KiB LoROM bank 정렬
* Internal title `RICOCHETANGLES S0`
* LoROM / SlowROM (`0x20`)
* ROM only (`0x00`)
* SRAM 없음 (`0x00`)
* Japan/SFC destination (`0x00`)
* Header ROM size와 실제 파일 크기 일치
* Checksum / complement 및 실제 ROM byte sum
* LoROM reset vector sanity

## MesenCE 실행

먼저 Build + Verify를 통과한 뒤 실행합니다.

```powershell
.\scripts\run-mesen.ps1
```

wrapper가 실제로 호출하는 형식은 다음과 같습니다.

```text
C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe <ABSOLUTE_ROM_PATH>
```

S0-01에서 MesenCE 창 제목 `MesenCE - ricochetangles_s0_hello`와 다음 화면 출력을 직접 확인했습니다.

```text
RICOCHETANGLES SFC
S0 HELLO
PVSNESLIB 4.6.0
BUILD S0-01
```

동일한 clean build를 연속 두 번 실행했을 때 ROM SHA-256도 일치했습니다.

```text
48cb7cbedcf9095f9a9b8c62334b46ed0c99fe798f59498b1f2b6fd82780479a
```

## S0-02 P1 Pad Raw Input

2026-08-23에 PVSnesLib 4.6.0의 `padsCurrent(0)`으로 P1 Standard Pad의 held/current 상태를 매 프레임 읽는 것을 MesenCE에서 확인했습니다. ROM은 SNES logical input만 처리하며 PC 키보드 매핑은 포함하지 않습니다.

화면에는 4자리 raw mask와 `UP`, `DOWN`, `LEFT`, `RIGHT`, `A`, `B`, `X`, `Y`, `L`, `R`, `START`, `SELECT`의 독립 상태를 동시에 표시합니다.

| Logical input | 확인한 raw mask |
| --- | --- |
| UP | `0800` |
| DOWN | `0400` |
| LEFT | `0200` |
| RIGHT | `0100` |
| A | `0080` |
| B | `8000` |
| X | `0040` |
| Y | `4000` |
| L | `0020` |
| R | `0010` |
| START | `1000` |
| SELECT | `2000` |

필수 동시 입력도 각 bit가 함께 `1`로 표시되는 것을 확인했습니다.

| Host input | Logical result | Raw mask |
| --- | --- | --- |
| `W + D` | `UP + RIGHT` | `0900` |
| `W + A` | `UP + LEFT` | `0A00` |
| `Q + E` | `L + R` | `0030` |
| `W + Numpad5` | `UP + A` | `0880` |

각 입력을 놓으면 `RAW 0000`과 모든 logical bit `0`으로 돌아오는 것을 확인했습니다. 동일한 clean build를 연속 두 번 실행한 S0-02 ROM SHA-256은 다음과 같이 일치했습니다.

```text
9d4387535c5ca1e1a8d60317b4b87484d1425d7ae605af525fff66a41f70cd1a
```

## S0-03 P2 SNES Mouse Raw Input

2026-08-23에 MesenCE 2.2.1의 Port 1을 `SNES Controller`, Port 2를 `SNES Mouse`로 설정하고 PVSnesLib 4.6.0의 P2 Mouse raw input을 확인했습니다. ROM은 Windows mouse 좌표나 버튼을 직접 읽지 않으며 MesenCE가 변환한 SNES Mouse protocol만 처리합니다.

초기화는 `initMouse(MOUSE_SLOW)`을 사용하고 최소 한 번의 VBlank 뒤부터 `mouseConnect[1]`, `mouse_x[1]`, `mouse_y[1]`, `mousePressed[1]`, `mouseSensitivity[1]`을 읽습니다. P1은 기존 `padsCurrent(0)` raw mask 표시를 유지합니다.

화면 상태줄 형식은 다음과 같습니다.

```text
P0000 C1 X00+000 Y00+000 L0R0S0
```

* `P`: P1 current raw mask
* `C`: P2 Mouse 연결 상태
* `X`, `Y`: raw hexadecimal byte와 signed decimal delta
* `L`, `R`: 현재 held button 상태
* `S`: SNES Mouse sensitivity (`MOUSE_SLOW` 초기값 `0`)

사용자 수동 검증에서 오른쪽 이동은 X raw의 bit 7이 0이고 양의 DX, 왼쪽 이동은 bit 7이 1이고 음의 DX로 표시됐습니다. 아래쪽 이동은 Y raw의 bit 7이 0이고 양의 DY, 위쪽 이동은 bit 7이 1이고 음의 DY로 표시됐습니다. 정지하면 `X00+000 Y00+000`으로 복귀했습니다.

좌클릭과 우클릭은 각각 `L1`, `R1`로 독립 표시되고 해제하면 0으로 복귀했습니다. 두 버튼을 함께 누르면 `L1R1`로 표시됐습니다. P1 D-pad 또는 face/shoulder button을 누른 상태에서도 P2 상하좌우 이동과 좌우 button 입력이 동시에 갱신되는 것을 확인했습니다.

동일한 clean build를 연속 두 번 실행한 S0-03 ROM SHA-256은 다음과 같이 일치했습니다.

```text
5fce7e8a05a17cc8825d705b00f01f39514b5ecc2935424bbf1385dd717e862b
```

## S0-04 Final Regression & Cross-Emulator Validation

2026-08-23에 S0-03 Known Good Source를 변경하지 않고 최종 회귀검사를 수행했습니다. `scripts\build.ps1` clean build를 두 번 실행했으며 compiler warning/error 없이 두 번 모두 `ROM_VERIFY=PASS`였습니다.

```text
BUILD 1 SHA-256: 5fce7e8a05a17cc8825d705b00f01f39514b5ecc2935424bbf1385dd717e862b
BUILD 2 SHA-256: 5fce7e8a05a17cc8825d705b00f01f39514b5ecc2935424bbf1385dd717e862b
MATCH: YES
```

MesenCE 2.2.1에서 ROM boot, 화면 안정성, P1 12-button raw input, P1 복합입력, P2 Mouse 연결·방향·idle·좌우 button, P1+P2 동시 입력을 다시 확인했습니다. 최종 입력 회귀 결과는 사용자 확인을 포함해 `PASS / VERIFIED IN MESENCE / USER CONFIRMED`입니다.

Secondary Emulator는 사용자 검증을 마친 bsnes nightly로 확정했습니다.

```text
SECONDARY EMULATOR: bsnes nightly
DIRECTORY: C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly
EXECUTABLE: C:\Users\LunarGagarin\Documents\bsnes-windows\bsnes-nightly\bsnes.exe
ROM BOOT: PASS / USER CONFIRMED
P1 PAD: PASS / USER CONFIRMED
P2 SNES MOUSE: PASS / USER CONFIRMED
P1 + P2 SIMULTANEOUS: PASS / USER CONFIRMED
```

정확한 bsnes nightly build number 또는 build date는 확인되지 않아 기록하지 않습니다. MesenCE와 동일한 logical controller mapping으로 검증했습니다.

Delta iOS는 추가 호환성 참고 환경입니다. 동일 S0 ROM 계열의 ROM boot, 화면 출력과 virtual P1 SNES controller는 `PASS / USER CONFIRMED`입니다. 이번 환경에서 SNES Mouse를 사용할 수 없었으므로 P2 Mouse는 `UNKNOWN`입니다. Delta iOS는 실제 Super Famicom 검증이 아닙니다.

## 현재 검증 경계

* `PASS`: clean/repeatable build, `.sfc` 생성, ROM sanity check, MesenCE 2.2.1 부팅 및 S0-03 화면 출력
* `PASS`: P1 index 0의 12개 Standard Pad held/current 입력, raw mask, 해제 및 동시 입력
* `PASS`: P2 index 1의 SNES Mouse 검출, raw X/Y, signed DX/DY, sensitivity 및 좌/우 held button 입력
* `PASS`: P1 Pad와 P2 Mouse의 이동 및 button 동시 입력
* `PASS`: S0-04 MesenCE 최종 회귀 및 동일 SHA-256의 clean build 2회
* `PASS / USER CONFIRMED`: bsnes nightly ROM boot, P1 Pad, P2 SNES Mouse 및 P1+P2 동시 입력
* `PASS / USER CONFIRMED`: Delta iOS ROM boot, 화면 출력 및 virtual P1 controller 보조 교차 호환성
* `UNKNOWN`: Delta iOS P2 SNES Mouse — 현재 환경에서 사용할 수 없음
* `UNVERIFIED`: 실제 Super Famicom 하드웨어

S0는 2026-08-23 사용자 GO 결정으로 `PASS / CLOSED`됐습니다. 이 시점의 ROM/build/input 구현과 위 SHA-256은 `KNOWN GOOD S0 BASELINE`이며, 이후 Gate에서 Boot/Input 회귀가 발생하면 이 상태와 비교합니다. 정상 동작하는 S0 build/input 구조는 새 구현을 이유로 불필요하게 재작성하지 않습니다.

## S1-01 Hull Movement V0

2026-08-23에 P1 `padsCurrent(0)` D-pad를 desired heading으로 변환하고, 제한 선회·가속·관성·감속으로 단일 hull placeholder를 움직이는 S1-01을 구현했습니다. 위치와 속도는 Q8.8 fixed-point, heading은 clockwise-positive `u8` 0~255를 사용합니다. 현재 tuning 값은 `MAX_SPEED=0x0180`, `ACCELERATION=0x0008`, `DECELERATION=0x0006`, `TURN_RATE=2`이며 확정 밸런스가 아닙니다.

8방향 heading은 `RIGHT=0`, `DOWN-RIGHT=32`, `DOWN=64`, `DOWN-LEFT=96`, `LEFT=128`, `UP-LEFT=160`, `UP=192`, `UP-RIGHT=224`입니다. 위치는 desired vector가 아니라 현재 hull heading의 integer sin/cos lookup 결과와 scalar speed로 갱신합니다. 입력을 놓으면 추가 steering을 멈추고 현재 방향으로 감속하며, 별도 reverse나 strafe는 없습니다.

화면에는 녹색 16x16 OBJ hull과 현재 heading 앞쪽의 노란 8x8 marker를 표시합니다. hull 중심은 진단 HUD 아래의 `X=16..239`, `Y=80..207` 범위로 clamp됩니다. compact 상태줄에는 P1 raw, throttle, position, current/target heading, speed와 P2 connection/raw X/raw Y/button mask를 표시합니다. P2 Mouse는 gameplay에 사용하지 않고 S0 regression 진단으로만 polling합니다.

clean build를 두 번 실행했고 두 번 모두 compiler warning/error 없이 `ROM_VERIFY=PASS`였으며 SHA-256이 일치했습니다.

```text
e881273477ff705b09eff27d064dd633115cf8a9f14b80bcac1498f8cbadef67
```

MesenCE 2.2.1과 지정된 bsnes nightly에서 동일 ROM의 S1-01 화면, hull과 heading marker 부팅을 확인했습니다. MesenCE 화면에서는 P2 connection indicator `C1`도 확인했습니다.

```text
S1-01 PASS
USER CONFIRMED 2026-08-23
```

사용자가 MesenCE에서 `D` 유지 직선 가속, 해제 후 관성 감속, `D` 이동 중 `W` 90도 곡선, `D` 이동 중 `A` U-turn, `W+D` 대각 target, 선회 중 해제 시 추가 steering 정지, P1 이동 중 P2 Mouse raw/버튼 동시 갱신이 모두 정상임을 확인했습니다. D-pad 방향키 이외의 P1 버튼 raw 입력도 계속 동작하는 것을 확인했습니다. 이는 host mapping을 통한 사용자 테스트 결과이며 ROM은 PC keyboard를 직접 읽지 않습니다.

## S1-01R Tank Control Revision

S1-01의 Q8.8 position, integer sin/cos lookup, heading 기반 이동, 관성, boundary, OBJ placeholder와 P1/P2 polling을 유지하면서 D-pad 입력 모델만 전차식 조작으로 수정했습니다.

```text
UP    = forward throttle
DOWN  = reverse throttle
LEFT  = heading counter-clockwise
RIGHT = heading clockwise
```

Speed는 signed Q8.8이며 `MAX_FORWARD_SPEED=0x0180`, `MAX_REVERSE_SPEED=0x00C0`, `FORWARD_ACCELERATION=0x0008`, `REVERSE_ACCELERATION=0x0008`, `COAST_DECELERATION=0x0006`, `TURN_RATE=2`를 사용합니다. 전진과 후진 전환은 반드시 speed 0을 거치며, throttle이 없거나 UP+DOWN conflict일 때는 coast deceleration으로 0에 접근합니다. LEFT+RIGHT는 neutral turn이고 speed 0에서도 pivot turn이 가능합니다. 후진 중에도 marker는 실제 이동 방향이 아니라 hull front를 계속 표시합니다.

두 번의 clean build가 compiler warning/error 없이 `ROM_VERIFY=PASS`였고 동일 SHA-256을 생성했습니다.

```text
8f7c0685e2e1b5e3526c26701620df5a11b4fa70675f6663d2fe06c34ba46f4b
```

MesenCE 2.2.1과 지정된 bsnes nightly에서 동일 ROM의 `S1-01R TANK CONTROL` 화면, signed speed diagnostic, hull과 heading marker 부팅을 확인했습니다. P2 connection/raw/button/sensitivity polling도 compact diagnostic에 유지됩니다. 사용자가 MesenCE에서 전차식 전진·후진·회전 조작감이 만족스럽고 기존 Mouse 입력도 계속 동작한다고 확인했습니다.

```text
S1-01R PASS
USER CONFIRMED 2026-08-23
```

## S1-02 16-Direction Hull Presentation V0

S1-01R의 gameplay heading, Q8.8 movement와 여섯 tuning constant를 변경하지 않고 0~255 heading을 16방향 placeholder frame으로 표시합니다. Nearest-frame 계산은 `((heading + 8) >> 4) & 0x0F`이며 frame 00부터 0F까지 시계방향 22.5도 간격입니다. Diagnostic은 internal heading `H`와 visual frame `F`를 동시에 표시합니다.

Asset source는 [16-frame ASCII indexed sheet](assets/hull_placeholder/hull_16dir.txt)입니다. `scripts/build.ps1`이 전용 PowerShell generator로 128×32 indexed BMP를 조립한 뒤 PVSnesLib 4.6.0의 `gfx4snes 2.2.0`을 호출해 SNES 4bpp `.pic`과 `.pal`을 `build/work`에 생성합니다. 별도 GUI conversion이나 수동 generated binary 편집은 필요하지 않습니다.

Hull은 frame당 16×16, 4 tiles/128 bytes이며 전체 16 frames는 64 tiles/2,048 graphics bytes입니다. 16-color OBJ palette 중 transparent, dark green, yellow diagnostic marker, green, light-green front highlight의 5 entries를 사용합니다. 16 frames는 128×32 sheet의 16-tile-wide VRAM layout으로 한 번만 업로드되며, runtime은 OAM tile offset만 변경합니다. Hull OBJ 1개와 기존 yellow marker OBJ 1개를 유지합니다.

두 번의 clean build가 compiler warning/error 없이 `ROM_VERIFY=PASS`였고 동일 SHA-256을 생성했습니다.

```text
8c711e7991f33f7f9be0225dac2c1ba0cb638b1a4d326804ce551eeb5d6820bb
```

MesenCE 2.2.1과 지정된 bsnes nightly에서 동일 ROM의 `S1-02 HULL 16-DIR V0` 화면, hull/front highlight, heading marker와 `H/F` diagnostic 부팅을 확인했습니다. Asset pixel source와 generated SNES 4bpp roundtrip, cardinal/wrap mapping과 tile layout도 자동 검사로 통과했습니다. 사용자는 CW/CCW 양방향 완전 회전에서 `F00` 복귀, 이동 중 포함 frame 누락 없음, 복합 이동의 안정적인 방향 회전과 유지, 후진 및 후진 선회 sprite 안정성, 관성 중 회전, 이동 중 P1 버튼과 P2 Mouse 동시 입력을 모두 확인했습니다. 현재 상태는 다음과 같습니다.

```text
S1-02 PASS
USER CONFIRMED 2026-08-23
```

---

## S1-02R Hull Scale & Readability Revision

S1-02의 16방향 source와 nearest-frame 계산 `((heading + 8) >> 4) & 0x0F`을 유지하면서 각 frame을 결정적으로 2배 확대해 32×32 placeholder Hull로 표시합니다. PVSnesLib OBJ 구성은 `OBJ_SIZE16_L32`이며 Hull은 large 32×32 OBJ 1개, yellow front marker와 UP/DOWN/LEFT/RIGHT HUD는 small 16×16 OBJ 5개를 사용합니다. 16방향 Hull graphics는 256 tiles/8,192 bytes입니다. `gfx4snes -s 32`가 만든 4×4 frame block의 16-tile-wide OBJ layout에 맞춰 runtime tile base를 `(frame / 4) * 64 + (frame % 4) * 4`로 선택합니다.

최상단 HUD는 네 개의 16×16 둥근 사각형 버튼과 arrow pixel icon으로 구성됩니다. Idle은 짙은 녹색 배경과 밝은 녹색 테두리, pressed는 노란 배경과 밝은 노란 테두리 및 어두운 arrow로 반전됩니다. 각 `KEY_*` bit를 독립적으로 표시하므로 UP+DOWN 또는 LEFT+RIGHT conflict에서도 두 아이콘이 함께 pressed가 되고 `THR:N` 또는 `TURN:N`이 유지됩니다.

Diagnostic은 최상단 `THR/TURN`, 그 아래 `HDG/FRM/SPD`, `P1/POS`, compact `P2:C/U/V/B/S` 순서로 압축했습니다. Hull 시작 중심은 `(128, 144)`입니다. P1 logical mapping, P2 Mouse polling, 16방향 heading/frame mapping과 승인된 여섯 movement tuning value는 유지했습니다. 현재 유효 범위가 Q8.8 16-bit에 완전히 들어가므로 position backing은 816-tcc의 32-bit stack-local codegen 민감성을 피하도록 static WRAM `u16` Q8.8로 저장하며, 실제 속도·가속·선회·boundary 결과는 기존 계약과 같습니다.

두 번의 clean build가 warning/error 없이 같은 ROM을 생성했고 둘 다 `ROM_VERIFY=PASS`였습니다.

```text
100459bcd547174be004dfe5b2fe67db4c374bbaa08d6956b8229baca10f5a4a
```

MesenCE 2.2.1과 지정된 bsnes nightly에서 동일 ROM의 중앙 시작 위치, 32×32 Hull/front marker, 버튼형 HUD, compact P1/P2 diagnostic 부팅을 확인했습니다. 사용자는 수정 ROM에서 좌우 선회 시 32×32 Hull sprite가 더 이상 쪼개지지 않고 정상 표시되는 것을 확인했습니다. 현재 상태는 다음과 같습니다.

```text
S1-02R PASS
USER CONFIRMED 2026-08-23
```

---

## S2-01 Virtual Aim & Independent Turret V0

P2 index 1의 기존 `mouse_x[1]` / `mouse_y[1]` 상대 이동을 screen-space Virtual Aim Cursor에 누적합니다. `+X=RIGHT`, `+Y=DOWN`, `AIM_GAIN=1`, clamp는 `X=8..247`, `Y=64..215`, 초기 위치는 Hull 전방 48 px입니다. `abs(dx)+abs(dy)<5`에서는 이전 목표각을 유지합니다. P2 좌우 button과 sensitivity는 기존 raw diagnostic에만 남아 있으며 gameplay action은 없습니다.

Turret은 Hull과 별개의 `u8` world/screen heading과 target heading을 가집니다. Tank-to-Cursor vector는 32개의 midpoint tangent threshold와 5-step integer cross-product search로 octant를 결정한 뒤 quadrant를 재구성합니다. Runtime division과 floating point는 사용하지 않으며 자동 검사에서 `-64..64` vector의 최대 오차는 1 heading unit, cardinal heading은 정확히 일치했습니다. Turret은 shortest path를 프레임당 최대 `TURRET_TURN_RATE=4`로 추종하고 visual은 Hull과 같은 `((heading+8)>>4)&0x0F` nearest 16-direction mapping을 사용합니다.

Turret placeholder는 16×16, 16방향, 64 tiles/2,048 graphics bytes와 5색/32-byte OBJ palette입니다. Cursor는 16×16이며 기존 3,072-byte Input HUD sheet 안의 128 graphics bytes를 사용합니다. OAM은 Hull large OBJ 1개, Turret small OBJ 1개, Cursor small OBJ 1개로 player-related 3개이며 marker와 4개 P1 HUD를 포함한 전체 active OBJ는 8개입니다. Turret은 Hull 중심에 고정되고 priority 3, Hull은 priority 2입니다.

관련 VRAM word allocation은 Hull `0x0000`(8,192 bytes, tiles 0..255), HUD/Cursor `0x1000`(3,072 bytes, marker tile 320, cursor tile 322), Turret `0x1600`(2,048 bytes, tiles 352..415), BG graphics `0x3000`입니다. ROM asset은 Hull bank 05, HUD/Cursor bank 06, Turret bank 07에 배치하며 build가 source/runtime code와 PVSnesLib RTS routine을 bank 00에 유지하는지 검사합니다.

`scripts\build.ps1` clean build는 compiler warning/error 0, angle/LUT 검사, bank-layout 검사와 `ROM_VERIFY=PASS`를 통과했습니다. MesenCE 2.2.1과 지정된 bsnes nightly에서 Hull, Turret, Cursor와 diagnostic 화면의 ROM boot를 확인했습니다. 최종 상태는 다음과 같습니다.

```text
S2-01 PASS
USER CONFIRMED 2026-08-23
ROM SHA-256: 4bff8d00cef06f584bb4b5469a5f00692e94fe7541cc4cb543b2dbe017a415a3
REAL HARDWARE: UNVERIFIED
```

사용자는 Cursor 이동, 독립 Turret 동작, Hull 이동과 P1/P2 입력 회귀가 모두 정상이라고 확인했습니다. 따라서 S2-01은 `PASS / USER CONFIRMED 2026-08-23`입니다. 실제 Super Famicom은 계속 `UNVERIFIED`이며, 이 확인은 S2 Gate 종료나 S2-02 시작 승인이 아닙니다.

---

## S2-02 Main Gun Fire & Projectile V0

P2 SNES Mouse Left의 held 값 `mousePressed[1] & mouse_L`에서 release를 한 번 관찰한 뒤 0→1 rising edge만 발사 요청으로 사용합니다. 부팅 시 이미 눌린 상태는 발사로 처리하지 않으며, P2 Right는 기존 raw diagnostic 외 gameplay action이 없습니다.

Projectile은 동적 할당 없는 4-slot 정적 pool입니다. 각 slot은 `active`, Q8.8 `positionX/Y`, Q8.8 `velocityX/Y`, 발사 순간의 `heading`을 보관합니다. Muzzle은 Hull 중심에서 실제 현재 `turret.heading`의 forward vector로 16 px 전진한 위치이고, 기존 `sin256`/`cos256` LUT를 재사용해 4 px/frame 직선 속도를 정합니다. Turret target이나 Hull heading은 발사각으로 사용하지 않습니다. Cooldown은 18 frames이며 pool이 가득 차면 요청을 무시합니다.

포탄 표식은 기존 Input HUD sheet의 예약 영역에 생성한 bright yellow 8×8 visual입니다. 전역 OBJ 설정 때문에 투명 영역을 포함한 tile footprint는 16×16이며 포탄 하나당 small OBJ 1개입니다. 최대 4발일 때 Hull 1 + Turret 1 + Cursor 1 + Projectile 4 = player-side 7 OBJ이고, front marker 1 + P1 HUD 4까지 포함한 전체 active 최대치는 12 OBJ입니다.

화면 밖 정리는 중심 X `4..251`, gameplay Y `56..223`을 벗어나거나 Q8.8 덧셈이 넘치는 즉시 deactivate합니다. Build는 projectile edge/heading/LUT 속도/OAM 계약, 기존 S2 angle과 bank layout, ROM sanity를 검사합니다. MesenCE 2.2.1과 지정된 bsnes nightly에서 `S2-02`, Hull, Turret, Cursor, `GUN F0 CD00 SH0 H00`의 깨끗한 boot를 확인했고 bsnes 화면은 60 FPS를 표시했습니다.

```text
S2-02 IMPLEMENTED
USER PLAYTEST REQUIRED
ROM SHA-256: 761eba62d93adc5e344a16ae8dc173c15ccc6c5fabd47dfa826e89ba8d00270a
REAL HARDWARE: UNVERIFIED
```

사용자는 MesenCE에서 Hull/Turret 독립 발사 방향, Turret traverse 중 실제 포신 방향 발사, 이동/후진/pivot 중 발사, press-edge/held/cooldown, 8방향 진행과 muzzle 정렬을 직접 확인해야 합니다. 이 확인 전에는 S2-02가 PASS가 아니며 S2를 닫거나 S3로 진행하지 않습니다.

---

## S2-02A Alternate Aim Input / P2 Pad V0

기본 `AIM:M`은 기존 P2 SNES Mouse의 누적 Virtual Aim, Mouse Left 발사와 `AIM_GAIN=1`을 그대로 사용합니다. P1 SELECT press edge는 `AIM:P`와 상호 전환하며, Mode 변경 직후 발사 입력을 disarm해 현재 장치의 발사 버튼을 한 번 놓기 전에는 발사하지 않습니다. Port 2가 선택된 Mode와 다른 장치일 때 gameplay 입력은 0으로 차단합니다.

`AIM:P`는 `padsCurrent(1)`의 D-pad 각 bit를 두 축으로 독립 해석합니다. 목표각은 `RIGHT=00`, `DOWN-RIGHT=20`, `DOWN=40`, `DOWN-LEFT=60`, `LEFT=80`, `UP-LEFT=A0`, `UP=C0`, `UP-RIGHT=E0`입니다. `UP+DOWN`과 `LEFT+RIGHT`는 해당 축만 중립화하며, 두 축이 모두 중립이면 기존 target heading을 유지합니다. 표시기는 Hull 중심에서 target heading 방향 48 px에 두며 기존 Mouse `AimState`를 수정하지 않습니다.

Mouse Left와 P2 Pad B는 장치 독립적인 `fireHeld`로 변환된 뒤 같은 release/rising-edge, 18-frame cooldown, 현재 `turret.heading` snapshot과 4-slot Projectile pool을 사용합니다. Build는 8방향/상반축/idle hold/mode safety/공통 발사 계약과 기존 angle, projectile, bank layout, ROM sanity를 함께 검사합니다.

```text
S2-02A PASS / USER CONFIRMED 2026-08-25
REAL HARDWARE: UNVERIFIED
```

구현 직후 MesenCE에서 P2 Mouse 구성과 P2 Standard Controller 구성의 동일 ROM boot 및 장치 판별을 확인했고, bsnes nightly에서는 `S2-02A`, `AIM:M`, Hull/Turret/Cursor와 diagnostic 화면 부팅을 확인했습니다. 당시 비어 있던 P2 Pad host mapping 때문에 세부 조작은 `USER PLAYTEST REQUIRED`로 남겼습니다.

사용자는 2026-08-25 P1 SELECT의 `AIM:M`/`AIM:P` 전환, `AIM:P`의 P2 D-pad 8방향 Turret Aim과 P2 B Main Gun Fire를 직접 확인했습니다. 따라서 S2-02A는 `PASS / USER CONFIRMED 2026-08-25`이며, 사용자 GO 결정에 따라 S2는 `PASS / CLOSED / USER GO APPROVED 2026-08-25`입니다. P1+P2 동시 조작과 Aim Mode 전환 accidental-fire 방지는 이후에도 regression 항목으로 유지합니다. 사용자가 확인하지 않은 다른 세부 항목을 과거 시점의 `USER CONFIRMED`로 기록하지 않습니다.

---

## S3-01 Static Enemy Target & Projectile Hit V0

화면 오른쪽 `(208, 144)`에 heading `128`(LEFT), HP 3인 정지 Enemy Tank 1대를 배치합니다. Enemy는 기존 Player Hull 32×32 graphics와 palette를 그대로 재사용하는 large OBJ 1개이며 움직임, AI, Turret과 Fire가 없습니다. 추가 Enemy graphics와 palette data는 각각 0 bytes입니다.

Player Projectile이 이동한 뒤 중심점이 Enemy 중심 기준 half-width/height 13 px의 회전하지 않는 26×26 AABB 안에 있으면 shell을 즉시 비활성화하고 damage 1과 Hit Count 1을 정확히 한 번 적용합니다. HP 0이면 Enemy를 inactive로 만들고 OBJ를 숨깁니다. 유효 hit는 6-frame blink로 표시하며 `EN HP3 HIT00` diagnostic이 HP/누적 hit를 보여줍니다.

P1 START는 DEV/diagnostic reset입니다. Enemy 위치, heading, HP 3, active와 Hit Count 0을 복원하고 Player Projectile pool을 비웁니다. 이 입력은 최종 gameplay mapping이 아닙니다. Collision은 Aim Mode를 모르며 Mouse와 P2 Pad가 같은 Projectile path를 사용합니다. Armor Face, Impact Angle, Ricochet, Non-Penetration, Penetration과 Ammo Type은 구현하지 않았습니다.

```text
S3-01 IMPLEMENTED
USER PLAYTEST REQUIRED
REAL HARDWARE: UNVERIFIED
```

자동검증은 Enemy 초기 상태, 직선 hit, miss, shell 1개당 1 damage, 3-hit destruction, reset, 기존 shell speed 4, S2 Aim/Fire regression과 OAM 비중복을 검사합니다. 사용자가 Miss, Hit, 3-hit destruction, Reset, Mouse hit와 PAD2 hit를 직접 확인하기 전에는 S3-01을 PASS로 승격하지 않습니다.

두 번의 clean build가 compiler warning/error 0, `S3_TARGET_VERIFY=PASS`, `ROM_VERIFY=PASS`를 통과했고 동일한 262,144-byte ROM을 생성했습니다. SHA-256은 두 번 모두 `31bca17b059e1e57c7729f47f2662913b661755c9887152fcb07ac2e774d2d82`였으며 Bank 00은 1,624 bytes(4.96%)가 남았습니다. MesenCE와 bsnes nightly에서 `S3-01`, Player Hull/Turret/Cursor, Enemy Tank와 `EN HP3 HIT00`의 정상 boot/표시를 확인했고 bsnes는 60 FPS였습니다. Boot 시 active shell이 없으므로 새 ROM의 Projectile 표시, 실제 hit 결과와 조작 regression은 사용자 수동 검증 전까지 `UNVERIFIED`입니다.

---

# AI-Assisted Development

이 프로젝트의 연구 주제 중 하나는 다음과 같습니다.

> **2026년의 AI·에이전틱 코딩을 활용하여 실제 1990년대 16bit 하드웨어용 신작을 얼마나 효율적으로 제작할 수 있는가?**

가능한 작업은 자동화합니다.

예:

* Toolchain 관리
* C / 65816 코드
* ROM Build
* Input
* Graphics Conversion
* Palette Conversion
* Sprite Metadata
* Lookup Table
* Map Conversion
* Validation
* Regression Test
* Build Scripts
* Documentation

목표는 사람이 65816 Assembly와 SFC PPU를 처음부터 모두 숙달해야만 개발할 수 있는 구조를 만드는 것이 아닙니다.

---

# DROP 정책

다음 문제가 핵심 재미를 막는 수준으로 확인되면 개발을 종료할 수 있습니다.

* 차체/포탑 독립 조작이 실용적이지 않음
* SNES Mouse 조준이 충분히 정밀하지 않음
* 화면 크기 때문에 장갑각과 위협을 읽기 어려움
* Sprite / VRAM / CPU 제약이 핵심 전투를 크게 훼손함
* Asset Pipeline의 수작업 비용이 지나치게 큼
* Toolchain 유지 비용이 실험 가치보다 큼
* AI가 코드를 생성하더라도 실제 디버깅 비용이 지나치게 큼
* Emulator와 실제 하드웨어의 차이를 안정적으로 관리할 수 없음
* 핵심 게임성을 유지하려면 SFC 플랫폼의 의미가 사라질 정도로 타협해야 함

이 경우 결과는 다음과 같이 기록할 수 있습니다.

```text
EXPERIMENTAL RESULT

TECHNICALLY INTERESTING
BUT NOT VIABLE

→ DROP
```

---

# 개발 원칙

```text
BOOT
→ INPUT
→ MOVEMENT
→ TURRET
→ FIRE
→ RICOCHET
```

그 뒤에야 다음을 검토합니다.

```text
AMMO
→ FOCUS
→ PART DAMAGE
→ ENEMY
→ STAGE
→ BOSS
→ ART
→ AUDIO
→ PHYSICAL CARTRIDGE
→ DEDICATED CONTROLLER
```

현재 가장 중요한 목표는 단순합니다.

> **P1 차체와 P2 Mouse 포탑의 독립 조작이 SFC에서 성립하는지 확인합니다.**

---

# Documentation

Agent / Codex 작업 규칙은 저장소 루트의 다음 문서를 우선 확인합니다.

```text
AGENTS.md
```

SFC Experimental의 상위 연구·개발 방향은 다음 문서를 기준으로 관리합니다.

```text
RicochetAngles_SFC_Experimental_구상안_v0.1_20260823.md
```

향후 실제 기술 검증 결과가 축적되면 필요한 경우에만 다음과 같은 계약 문서를 추가합니다.

```text
SFC_TECH_CONTRACT_v0.1.md
SFC_INPUT_CONTRACT_v0.1.md
SFC_ASSET_PIPELINE_v0.1.md
```

문서는 실제 필요성이 확인된 뒤 추가합니다.

---

# Current Status

```text
PROJECT
RicochetAngles SFC Experimental

STATUS
EXPERIMENTAL / DROP-OK

CURRENT GATE
S3 — CORE COMBAT

CURRENT OBJECTIVE
Verify one static Enemy AABB receives exactly one damage
from each valid Player Projectile hit.

COMPLETED GATE
S2 — INDEPENDENT TURRET — PASS / CLOSED / USER GO APPROVED 2026-08-25

CURRENT STATUS
S3-01 — IMPLEMENTED / USER PLAYTEST REQUIRED

CURRENT SUBTASK
S3-01 — Static Enemy Target & Projectile Hit V0

Do not mark S3-01 PASS or proceed to S3-02, Armor Face, Impact Angle, Ricochet, Non-Penetration or Penetration without user confirmation.
```
