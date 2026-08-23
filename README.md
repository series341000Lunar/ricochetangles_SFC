# RicochetAngles — SFC Experimental

**현대의 생성형 AI·에이전틱 코딩·자동화 기술을 활용하여 실제 Super Famicom / SNES 하드웨어에서 동작하는 RicochetAngles 신작을 만들 수 있는지 검증하는 독립 Experimental 프로젝트입니다.**

> **현재 상태:** `S0 — BOOT & INPUT`
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

## S0 — BOOT & INPUT

현재 첫 번째 목표는 **게임을 만드는 것이 아니라 개발 환경이 실제로 성립하는지 검증하는 것**입니다.

현재 목표:

```text
SOURCE
→ BUILD
→ VALID .sfc ROM
→ PC EMULATOR BOOT
→ P1 PAD INPUT
→ P2 SNES MOUSE INPUT
```

### S0에서 확인할 것

* SFC Toolchain 구축
* 반복 가능한 `.sfc` ROM Build
* 정상적인 ROM Boot
* 최소 화면 출력
* P1 Standard Controller 입력
* P2 SNES Mouse 검출
* Mouse 상대 이동값
* Mouse Button 입력
* 최소 ROM Validation
* Build / Verify 자동화

### S0에서 하지 않는 것

현재 단계에서는 다음을 만들지 않습니다.

* 전차 이동
* 포탑
* 주포
* Projectile
* 장갑 판정
* 도탄
* Enemy
* Stage
* Boss
* 최종 Sprite
* 음악
* Flash Cartridge 대응
* Physical Cartridge
* 전용 Controller

먼저 **ROM을 안정적으로 띄울 수 있는지** 확인합니다.

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

## S0 — BOOT & INPUT

PC Emulator에서 정상적인 SFC ROM을 반복 빌드하고 기본 입력을 읽습니다.

핵심 질문:

> 우리가 안정적으로 SFC 프로그램을 만들 수 있는가?

---

## S1 — DRIVE

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

## S2 — INDEPENDENT TURRET

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

## 현재 검증 경계

* `PASS`: clean/repeatable build, `.sfc` 생성, ROM sanity check, MesenCE 2.2.1 부팅 및 화면 출력
* `UNVERIFIED`: Secondary Emulator 교차 부팅
* `UNVERIFIED`: 실제 Super Famicom 하드웨어
* 미구현: P1 Controller 및 P2 SNES Mouse 입력 — S0-01 범위 밖이며 다음 작업에서 별도 검증

S0-01은 완료됐지만 전체 S0 Gate는 아직 닫히지 않았습니다.

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

> **먼저 실제 SFC ROM을 띄웁니다.**

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
S0 — BOOT & INPUT

CURRENT OBJECTIVE
Build and boot a valid SFC ROM,
then verify P1 Pad and P2 SNES Mouse input.

NEXT
S1 — DRIVE

S1 starts only after S0 verification and GO decision.
```
