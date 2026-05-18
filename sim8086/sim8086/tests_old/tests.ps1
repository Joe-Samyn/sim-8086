

param(
    [string]$SimulatorPath,
    [string]$TestFile
)

$TestFileAbsolutePath = Resolve-Path -Path $TestFile

$currentDir = Get-Location

$ResultAsm = "$currentDir/result.asm"
$ResultBin = "$currentDir/result.out"


Write-Host "Running tests for Intel 8086 simulator..."
Write-Host "Simulator Path: $SimulatorPath"
Write-Host "Test File: $TestFile"

# Check if the simulator executable exists
if (-Not (Test-Path -Path $SimulatorPath -PathType Leaf)) {
    Write-Error "Simulator executable not found at path: $SimulatorPath"
    exit 1
}

# Check if the test file exists 
if (-Not (Test-Path -Path $TestFileAbsolutePath -PathType Leaf)) {
    Write-Error "Test file was not found at path: $TestFileAbsolutePath"
    exit 1
}

# Run the similator 
# TODO: Add error handling for simulator execution
& $SimulatorPath $TestFileAbsolutePath

# Assembly test file 
nasm $ResultAsm -o $ResultBin

# Read file contents for comparing
$fileOne = [System.IO.File]::ReadAllBytes($ResultBin)
$fileTwo = [System.IO.File]::ReadAllBytes($TestFileAbsolutePath)

# Compare the output with expected output
Compare-Object $fileOne $fileTwo