ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "PVSNESLIB_HOME is required; run scripts/build.ps1 from PowerShell")
endif

export ROMNAME      := ricochetangles_s0_hello
export ROMTITLE     := RICOCHETANGLES S0
export CARTRIDGETYPE := 00
export ROMSIZE      := 08
export ROMBANKS     := 08
export SRAMSIZE     := 00
export COUNTRY      := 00
export LICENSEECODE := 00
export VERSION      := 00
export HIROM        := 0
export FASTROM      := 0

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all clean cleanLogs

all: buildWithSummary
buildActual: $(OBJS) $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanLogs

