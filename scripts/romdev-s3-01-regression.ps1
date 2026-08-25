[CmdletBinding()]
param(
    [ValidateSet('Prepare', 'Validate')]
    [string]$Mode = 'Prepare',
    [string]$DefinitionPath = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRootOutput = & git rev-parse --show-toplevel
if ($LASTEXITCODE -ne 0) {
    throw 'Unable to resolve the Git repository root.'
}

$repoRoot = [System.IO.Path]::GetFullPath(($repoRootOutput | Select-Object -First 1).Trim())
$repoPrefix = $repoRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar

function Resolve-RepositoryPath {
    param([Parameter(Mandatory)][string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        $fullPath = [System.IO.Path]::GetFullPath($Path)
    }
    else {
        $fullPath = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
    }

    if (-not $fullPath.StartsWith($repoPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Path escapes the repository: $fullPath"
    }

    return $fullPath
}

function Get-RequiredPropertyValue {
    param(
        [Parameter(Mandatory)]$Object,
        [Parameter(Mandatory)][string]$Name
    )

    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "Required property is missing: $Name"
    }

    return $property.Value
}

function Write-Utf8Json {
    param(
        [Parameter(Mandatory)]$Value,
        [Parameter(Mandatory)][string]$Path
    )

    $json = $Value | ConvertTo-Json -Depth 32
    $utf8WithoutBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $json + [System.Environment]::NewLine, $utf8WithoutBom)
}

if ([string]::IsNullOrWhiteSpace($DefinitionPath)) {
    $DefinitionPath = 'regressions\romdev\s3-01-smoke.json'
}

$definitionPathFull = Resolve-RepositoryPath -Path $DefinitionPath
if (-not (Test-Path -LiteralPath $definitionPathFull -PathType Leaf)) {
    throw "Regression definition is missing: $definitionPathFull"
}

$definition = Get-Content -LiteralPath $definitionPathFull -Encoding UTF8 -Raw | ConvertFrom-Json
if ([int]$definition.schemaVersion -ne 1) {
    throw "Unsupported regression definition schema: $($definition.schemaVersion)"
}
if ([string]$definition.platform -ne 'snes') {
    throw "Unexpected regression platform: $($definition.platform)"
}

$romPath = Resolve-RepositoryPath -Path ([string]$definition.romPath)
$symbolPath = Resolve-RepositoryPath -Path ([string]$definition.symbolPath)
if (-not (Test-Path -LiteralPath $romPath -PathType Leaf)) {
    throw "Authoritative build ROM is missing: $romPath"
}
if (-not (Test-Path -LiteralPath $symbolPath -PathType Leaf)) {
    throw "Authoritative build symbol file is missing: $symbolPath"
}

$artifacts = $definition.artifacts
$artifactDirectory = Resolve-RepositoryPath -Path ([string]$artifacts.directory)
$resolvedPath = Resolve-RepositoryPath -Path ([string]$artifacts.resolved)
$captureArgsPath = Resolve-RepositoryPath -Path ([string]$artifacts.captureArgs)
$checkArgsPath = Resolve-RepositoryPath -Path ([string]$artifacts.checkArgs)
$goldenPath = Resolve-RepositoryPath -Path ([string]$artifacts.golden)
$runAScreenshotPath = Resolve-RepositoryPath -Path ([string]$artifacts.runAScreenshot)
$runBScreenshotPath = Resolve-RepositoryPath -Path ([string]$artifacts.runBScreenshot)
New-Item -ItemType Directory -Path $artifactDirectory -Force | Out-Null

$requiredSymbolNames = @($definition.symbols.PSObject.Properties | ForEach-Object { [string]$_.Value })
$symbolAddresses = @{}
foreach ($line in Get-Content -LiteralPath $symbolPath) {
    if ($line -match '^([0-9A-Fa-f]{8})\s+(.+)$') {
        $symbolName = $Matches[2]
        if ($requiredSymbolNames -notcontains $symbolName) {
            continue
        }
        if ($symbolAddresses.ContainsKey($symbolName)) {
            throw "Required symbol is duplicated in linker output: $symbolName"
        }
        $symbolAddresses[$symbolName] = [Convert]::ToUInt32($Matches[1], 16)
    }
}

$resolvedSymbols = [ordered]@{}
foreach ($symbolProperty in $definition.symbols.PSObject.Properties) {
    $symbolAlias = $symbolProperty.Name
    $symbolName = [string]$symbolProperty.Value
    if (-not $symbolAddresses.ContainsKey($symbolName)) {
        throw "Required linker symbol is missing: $symbolName"
    }

    $cpuAddress = [uint32]$symbolAddresses[$symbolName]
    if ($cpuAddress -lt 0x007E0000 -or $cpuAddress -gt 0x007FFFFF) {
        throw ('Symbol {0} is outside SNES WRAM: 0x{1:X6}' -f $symbolName, $cpuAddress)
    }

    $resolvedSymbols[$symbolAlias] = [ordered]@{
        name = $symbolName
        cpuAddress = ('0x{0:X6}' -f $cpuAddress)
        systemRamOffset = [int]($cpuAddress - 0x007E0000)
    }
}

$resolvedFields = [ordered]@{}
foreach ($fieldProperty in $definition.fields.PSObject.Properties) {
    $fieldName = $fieldProperty.Name
    $fieldDefinition = $fieldProperty.Value
    $symbolAlias = [string]$fieldDefinition.symbol
    if (-not $resolvedSymbols.Contains($symbolAlias)) {
        throw "Field $fieldName references an unknown symbol alias: $symbolAlias"
    }

    $symbolRecord = $resolvedSymbols[$symbolAlias]
    $relativeOffset = [int]$fieldDefinition.relativeOffset
    $length = [int]$fieldDefinition.length
    if ($relativeOffset -lt 0 -or $length -lt 1) {
        throw "Invalid field range for $fieldName"
    }

    $systemRamOffset = [int]$symbolRecord.systemRamOffset + $relativeOffset
    if ($systemRamOffset + $length -gt 0x20000) {
        throw "Field $fieldName escapes SNES WRAM."
    }

    $resolvedFields[$fieldName] = [ordered]@{
        symbol = $symbolRecord.name
        symbolCpuAddress = $symbolRecord.cpuAddress
        relativeOffset = $relativeOffset
        region = 'system_ram'
        offset = $systemRamOffset
        length = $length
    }
}

$nativeCheckpoints = @()
$expectations = @()
foreach ($checkpoint in $definition.checkpoints) {
    $memory = @()
    foreach ($assertion in $checkpoint.assertions) {
        $fieldName = [string]$assertion.field
        if (-not $resolvedFields.Contains($fieldName)) {
            throw "Checkpoint $($checkpoint.label) references an unknown field: $fieldName"
        }

        $field = $resolvedFields[$fieldName]
        $memory += [ordered]@{
            label = $fieldName
            region = $field.region
            offset = [int]$field.offset
            length = [int]$field.length
        }
        $expectations += [ordered]@{
            checkpoint = [string]$checkpoint.label
            frame = [int]$checkpoint.frame
            field = $fieldName
            expectedHex = ([string]$assertion.expectedHex).ToUpperInvariant()
        }
    }

    $nativeCheckpoints += [ordered]@{
        frame = [int]$checkpoint.frame
        label = [string]$checkpoint.label
        observe = @('memory')
        memory = @($memory)
    }
}

$captureArgs = [ordered]@{
    op = 'capture'
    goldenPath = $goldenPath
    inputScript = @($definition.inputScript)
    checkpoints = @($nativeCheckpoints)
}
$checkArgs = [ordered]@{
    op = 'check'
    goldenPath = $goldenPath
}
$resolved = [ordered]@{
    schemaVersion = 1
    name = [string]$definition.name
    definitionPath = $definitionPathFull
    platform = [string]$definition.platform
    romPath = $romPath
    symbolPath = $symbolPath
    goldenPath = $goldenPath
    screenshots = [ordered]@{
        runA = $runAScreenshotPath
        runB = $runBScreenshotPath
    }
    symbols = $resolvedSymbols
    fields = $resolvedFields
    expectations = @($expectations)
    captureArgsPath = $captureArgsPath
    checkArgsPath = $checkArgsPath
}

Write-Utf8Json -Value $resolved -Path $resolvedPath
Write-Utf8Json -Value $captureArgs -Path $captureArgsPath
Write-Utf8Json -Value $checkArgs -Path $checkArgsPath

Write-Output 'S3_01_ROMDEV_RESOLVE=PASS'
Write-Output "ROM=$romPath"
Write-Output "SYMBOLS=$symbolPath"
foreach ($fieldName in $resolvedFields.Keys) {
    $field = $resolvedFields[$fieldName]
    Write-Output ('FIELD_{0}=system_ram+0x{1:X4} ({2}+{3})' -f $fieldName.ToUpperInvariant(), [int]$field.offset, $field.symbol, [int]$field.relativeOffset)
}
Write-Output "CAPTURE_ARGS=$captureArgsPath"
Write-Output "CHECK_ARGS=$checkArgsPath"
Write-Output "GOLDEN=$goldenPath"

if ($Mode -eq 'Validate') {
    if (-not (Test-Path -LiteralPath $goldenPath -PathType Leaf)) {
        throw "romdev golden is missing: $goldenPath"
    }

    $golden = Get-Content -LiteralPath $goldenPath -Encoding UTF8 -Raw | ConvertFrom-Json
    foreach ($expectation in $expectations) {
        $checkpointMatches = @($golden.checkpoints | Where-Object { [string]$_.label -eq [string]$expectation.checkpoint })
        $observationMatches = @($golden.observations | Where-Object { [string]$_.label -eq [string]$expectation.checkpoint })
        if ($checkpointMatches.Count -ne 1 -or $observationMatches.Count -ne 1) {
            throw "Golden checkpoint is missing or duplicated: $($expectation.checkpoint)"
        }

        $field = $resolvedFields[[string]$expectation.field]
        $memoryMatches = @($checkpointMatches[0].memory | Where-Object { [string]$_.label -eq [string]$expectation.field })
        if ($memoryMatches.Count -ne 1) {
            throw "Golden memory field is missing or duplicated: $($expectation.checkpoint)/$($expectation.field)"
        }
        if ([string]$memoryMatches[0].region -ne [string]$field.region -or
            [int]$memoryMatches[0].offset -ne [int]$field.offset -or
            [int]$memoryMatches[0].length -ne [int]$field.length) {
            throw "Golden memory address is stale: $($expectation.checkpoint)/$($expectation.field)"
        }

        $actualProperty = $observationMatches[0].obs.memory.PSObject.Properties[[string]$expectation.field]
        if ($null -eq $actualProperty) {
            throw "Golden observation is missing: $($expectation.checkpoint)/$($expectation.field)"
        }
        $actualHex = ([string]$actualProperty.Value).ToUpperInvariant()
        if ($actualHex -ne [string]$expectation.expectedHex) {
            throw "Gameplay assertion failed at $($expectation.checkpoint)/$($expectation.field): expected $($expectation.expectedHex), actual $actualHex"
        }

        Write-Output ("ASSERT_{0}_{1}=PASS expected={2} actual={3}" -f
            ([string]$expectation.checkpoint).ToUpperInvariant(),
            ([string]$expectation.field).ToUpperInvariant(),
            [string]$expectation.expectedHex,
            $actualHex)
    }

    Write-Output 'S3_01_ROMDEV_GAMEPLAY_ASSERTIONS=PASS'
}
