# romdev Agent Regression Harness

romdev는 이 저장소에서 `scripts/build.ps1`이 생성한 ROM을 대상으로 하는 headless runtime regression / memory debug 도구다. romdev compiler 또는 build 기능은 사용하지 않는다.

## S3-02R1 Heavy Target silhouette gameplay-preservation smoke

S3-02R1은 visual asset만 수정하므로 S3-02R의 PEN / RIC / HP20 runtime checkpoints를 그대로 재사용한다.

Source definition:

```text
regressions/romdev/s3-01-smoke.json
```

이 정의는 input sequence, checkpoint, 기대 gameplay 값과 linker symbol 이름을 보관한다. 영구 raw WRAM absolute address는 보관하지 않는다.

## WRAM address policy

`scripts/romdev-s3-01-regression.ps1`은 clean build의 `build/work/ricochetangles_s0_hello.sym`에서 `enemy`, `lastArmorImpact`, `enemyHitCount`, `fireCooldown`의 WRAM 주소를 매번 다시 찾는다. SNES CPU address `0x7E0000..0x7FFFFF`만 허용하고 이를 romdev `system_ram` offset으로 변환한다. generated golden의 주소가 fresh symbol resolution과 다르면 `-Mode Validate`가 실패한다.

현재 linker output은 struct 내부 field symbol을 제공하지 않는다. 따라서 `enemy_hp`, `enemy_max_hp`, `enemy_active`, `enemy_hit_flash`, `enemy_class_id`는 동적으로 찾은 `enemy` base에 현재 `EnemyState` layout의 상대 offset `+5`, `+6`, `+7`, `+8`, `+9`를 적용하고, armor face/angle/result는 동적으로 찾은 `lastArmorImpact` base에 `+4`, `+6`, `+7`을 적용한다. 이는 raw absolute WRAM address보다 안전하지만 field-level linker symbol만큼 강하지는 않다. Struct layout이 자주 바뀌거나 더 많은 runtime assertion이 필요해지면 고정 debug telemetry WRAM block을 별도 제안한다. 이번 harness는 gameplay code 또는 WRAM layout을 변경하지 않는다.

실행 순서:

1. `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1`
2. `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\romdev-s3-01-regression.ps1 -Mode Prepare`
3. romdev `loadMedia`로 resolved metadata의 `romPath`를 clean load한다.
4. generated `build/romdev/s3-01/capture-args.json`을 romdev native `regression` call에 전달한다.
5. frame 1100 framebuffer를 resolved metadata의 Run A screenshot 경로에 저장한다. Screenshot은 gameplay hard assertion에 포함하지 않는다.
6. `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\romdev-s3-01-regression.ps1 -Mode Validate`로 explicit gameplay expectations와 fresh symbol address를 검증한다.
7. 같은 ROM을 clean load하고 generated `build/romdev/s3-01/check-args.json`으로 native regression check를 실행한다.
8. frame 1100 framebuffer를 Run B screenshot 경로에 저장하고 두 screenshot을 별도 diagnostic artifact로 비교한다.

Native regression의 hard result는 memory checkpoint다. Run B는 Run A golden과 같아야 하며, Run A golden 자체도 source definition의 다음 값과 일치해야 한다.

```text
initial frame 50: class HEAVY_TANK_TEST, active, HP/max HP 20, hit count 0
hit frame 140: HP 19, hit count 1, FRONT, angle 0, PENETRATION
final frame 1100: HP 0, inactive, hit count 20, final FRONT/0/PEN result, hit flash 0, cooldown 0
```

P1 menu와 P2 B input은 PVSnesLib polling frame에 확실히 걸리도록 5-frame hold/release와 50-frame shot interval을 사용한다. RICOCHET의 HP 불변은 별도 headless WRAM fixture에서 HP 19 상태에 heading `36` hex(54 decimal) Projectile을 주입하여 `FRONT / angle 36 / RICOCHET`, HP 19, hit count 1을 확인한다. 이 주입은 armor math를 우회하지 않고 기존 `updateProjectiles()` collision path만 실행한다.

`build/romdev/` 아래 resolved request, golden과 screenshot은 generated artifact이며 Git source of truth가 아니다.
