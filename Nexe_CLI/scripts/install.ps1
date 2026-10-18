<#
Build the native Nexe engine on Windows AMD64 or ARM64.
Run from PowerShell: .\install.ps1
#>
[CmdletBinding()]
param(
    [ValidateSet("native", "amd64", "aarch64")]
    [string]$Architecture = "native",
    [string]$BuildDirectory = "build"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake is required. Install it from https://cmake.org/download/."
}

# Visual Studio generators select Windows targets with -A. Native is left to CMake.
$cmakeArguments = @("-S", $projectRoot, "-B", (Join-Path $projectRoot $BuildDirectory), "-DNEXE_TARGET_ARCH=$Architecture")
if ($Architecture -eq "amd64") { $cmakeArguments += @("-A", "x64") }
if ($Architecture -eq "aarch64") { $cmakeArguments += @("-A", "ARM64") }

Write-Host "Configuring Nexe for $Architecture..."
& cmake @cmakeArguments
Write-Host "Building Nexe..."
& cmake --build (Join-Path $projectRoot $BuildDirectory) --config Release
Write-Host "Nexe engine build complete."
