<#
.SYNOPSIS
    DevEnvironmentCache - Detects and caches the presence of compiler languages, 
    runtimes, and package managers.
.DESCRIPTION
    This script inspects the host system for common programming environments.
    To optimize performance, it caches the results into a local JSON file.
    If the cache is valid and not expired, it loads data from the cache instead 
    of re-running expensive system path lookups.
.EXAMPLE
    .\DevEnvironmentCache.ps1 -ForceRefresh
#>

[CmdletBinding()]
param (
    [Parameter(Mandatory = $false)]
    [string]$CacheFilePath = "$env:USERPROFILE\.dev_env_cache.json",

    [Parameter(Mandatory = $false)]
    [int]$CacheExpiryHours = 24,

    [Parameter(Mandatory = $false)]
    [switch]$ForceRefresh
)

# -------------------------------------------------------------------------
# GLOBAL CONSTANTS & STRUCTURES
# -------------------------------------------------------------------------
$global:EnvironmentManifest = [ordered]@{
    "Languages"       = @{}
    "PackageManagers" = @{}
    "BuildTools"      = @{}
}

# -------------------------------------------------------------------------
# HELPER FUNCTIONS
# -------------------------------------------------------------------------
function Test-CommandAvailable {
    param ([string]$Command)
    $null -eq (Get-Command $Command -ErrorAction SilentlyContinue) ? $false : $true
}

function Get-CommandVersion {
    param (
        [string]$Command,
        [string]$Arguments = "--version",
        [string]$RegexMatch = "(\d+\.\d+\.\d+)"
    )
    
    if (-not (Test-CommandAvailable -Command $Command)) {
        return $null
    }

    try {
        $output = &$Command $Arguments 2>&1 | Out-String
        if ($output -match $RegexMatch) {
            return $Matches[1].Trim()
        }
        return "Detected (Version unknown)"
    }
    catch {
        return "Error reading version"
    }
}

function Register-EnvironmentItem {
    param (
        [ValidateSet("Languages", "PackageManagers", "BuildTools")]
        [string]$Category,
        [string]$Name,
        [bool]$Present,
        [string]$Version = $null,
        [string]$Path = $null
    )

    $itemPath = $Path
    if ($Present -and [string]::IsNullOrEmpty($itemPath)) {
        $itemPath = (Get-Command $Name -ErrorAction SilentlyContinue).Source
    }

    $global:EnvironmentManifest[$Category][$Name] = [ordered]@{
        "Present"    = $Present
        "Version"    = if ($Present) { $Version } else { $null }
        "Path"       = $itemPath
        "DetectedAt" = (Get-Date).ToString("o")
    }
}

# -------------------------------------------------------------------------
# CORE DETECTION ENGINE
# -------------------------------------------------------------------------
function Invoke-EnvironmentDetection {
    Write-Host "[*] Scanning system for compiler languages and tools..." -ForegroundColor Cyan

    # --- 1. COMPILER LANGUAGES & RUNTIMES ---
    
    # Python
    $pyVer = Get-CommandVersion -Command "python" -RegexMatch "Python\s+([\d\.]+)"
    if ($null -eq $pyVer) { $pyVer = Get-CommandVersion -Command "py" }
    Register-EnvironmentItem -Category "Languages" -Name "Python" -Present ($null -ne $pyVer) -Version $pyVer

    # Node.js
    $nodeVer = Get-CommandVersion -Command "node" -Arguments "-v" -RegexMatch "v([\d\.]+)"
    Register-EnvironmentItem -Category "Languages" -Name "NodeJS" -Present ($null -ne $nodeVer) -Version $nodeVer

    # Go (Golang)
    $goVer = Get-CommandVersion -Command "go" -Arguments "version" -RegexMatch "goversion\s+go([\d\.]+)"
    Register-EnvironmentItem -Category "Languages" -Name "Go" -Present ($null -ne $goVer) -Version $goVer

    # Rust
    $rustVer = Get-CommandVersion -Command "rustc"
    Register-EnvironmentItem -Category "Languages" -Name "Rust" -Present ($null -ne $rustVer) -Version $rustVer

    # Java (JDK)
    $javaVer = Get-CommandVersion -Command "java" -Arguments "-version" -RegexMatch "version\s+""([\d\._]+)"""
    Register-EnvironmentItem -Category "Languages" -Name "Java" -Present ($null -ne $javaVer) -Version $javaVer

    # .NET SDK
    $dotnetVer = Get-CommandVersion -Command "dotnet" -Arguments "--version"
    Register-EnvironmentItem -Category "Languages" -Name "DotNetSDK" -Present ($null -ne $dotnetVer) -Version $dotnetVer

    # C++ (GCC / G++)
    $gccVer = Get-CommandVersion -Command "g++" -RegexMatch "g\+\+\s+.*([\d\.]+)"
    Register-EnvironmentItem -Category "Languages" -Name "GCC_CPP" -Present ($null -ne $gccVer) -Version $gccVer

    # --- 2. PACKAGE MANAGERS ---

    # Pip (Python)
    $pipVer = Get-CommandVersion -Command "pip" -RegexMatch "pip\s+([\d\.]+)"
    Register-EnvironmentItem -Category "PackageManagers" -Name "Pip" -Present ($null -ne $pipVer) -Version $pipVer

    # NPM (Node)
    $npmVer = Get-CommandVersion -Command "npm" -Arguments "-v"
    Register-EnvironmentItem -Category "PackageManagers" -Name "NPM" -Present ($null -ne $npmVer) -Version $npmVer

    # Yarn
    $yarnVer = Get-CommandVersion -Command "yarn" -Arguments "-v"
    Register-EnvironmentItem -Category "PackageManagers" -Name "Yarn" -Present ($null -ne $yarnVer) -Version $yarnVer

    # Cargo (Rust)
    $cargoVer = Get-CommandVersion -Command "cargo"
    Register-EnvironmentItem -Category "PackageManagers" -Name "Cargo" -Present ($null -ne $cargoVer) -Version $cargoVer

    # NuGet (.NET)
    $nugetVer = Get-CommandVersion -Command "nuget" -RegexMatch "NuGet\s+Version:\s+([\d\.]+)"
    Register-EnvironmentItem -Category "PackageManagers" -Name "NuGet" -Present ($null -ne $nugetVer) -Version $nugetVer

    # Chocolatey (Windows)
    $chocoVer = Get-CommandVersion -Command "choco" -Arguments "-v"
    Register-EnvironmentItem -Category "PackageManagers" -Name "Chocolatey" -Present ($null -ne $chocoVer) -Version $chocoVer

    # Scoop (Windows alternative)
    $scoopVer = Get-CommandVersion -Command "scoop" -Arguments "-v" -RegexMatch "([\d\.]+)"
    Register-EnvironmentItem -Category "PackageManagers" -Name "Scoop" -Present ($null -ne $scoopVer) -Version $scoopVer

    # --- 3. BUILD TOOLS ---

    # Git
    $gitVer = Get-CommandVersion -Command "git" -RegexMatch "git\s+version\s+([\d\.]+)"
    Register-EnvironmentItem -Category "BuildTools" -Name "Git" -Present ($null -ne $gitVer) -Version $gitVer

    # Docker
    $dockerVer = Get-CommandVersion -Command "docker" -Arguments "-v" -RegexMatch "version\s+([\d\.]+)"
    Register-EnvironmentItem -Category "BuildTools" -Name "Docker" -Present ($null -ne $dockerVer) -Version $dockerVer

    # CMake
    $cmakeVer = Get-CommandVersion -Command "cmake" -RegexMatch "cmake\s+version\s+([\d\.]+)"
    Register-EnvironmentItem -Category "BuildTools" -Name "CMake" -Present ($null -ne $cmakeVer) -Version $cmakeVer

    # Make
    $makeVer = Get-CommandVersion -Command "make" -RegexMatch "GNU\s+Make\s+([\d\.]+)"
    Register-EnvironmentItem -Category "BuildTools" -Name "Make" -Present ($null -ne $makeVer) -Version $makeVer

    # Add metadata to payload
    $payload = [ordered]@{
        "CacheMetadata" = @{
            "GeneratedAt" = (Get-Date).ToString("o")
            "HostName"    = $env:COMPUTERNAME
            "OS"          = (Get-CimInstance Win32_OperatingSystem).Caption
        }
        "Data" = $global:EnvironmentManifest
    }

    return $payload
}

# -------------------------------------------------------------------------
# CACHE LIFE-CYCLE MANAGEMENT
# -------------------------------------------------------------------------
function Save-CacheToFile {
    param (
        [Parameter(Mandatory = $true)]
        $Data,
        [string]$Path
    )
    try {
        $jsonOutput = $Data | ConvertTo-Json -Depth 5
        $jsonOutput | Out-File -FilePath $Path -Encoding utf8 -Force
        Write-Host "[+] Environment cache successfully written to: $Path" -ForegroundColor Green
    }
    catch {
        Write-Warning "Failed to write cache file: $_"
    }
}

function Load-CacheFromFile {
    param ([string]$Path)
    try {
        $rawJson = Get-Content -Raw -Path $Path -ErrorAction SilentlyContinue
        if ([string]::IsNullOrEmpty($rawJson)) { return $null }
        return ConvertFrom-Json -InputObject $rawJson
    }
    catch {
        Write-Warning "Cache file corrupted or unreadable. Forcing refresh."
        return $null
    }
}

function Test-CacheValid {
    param (
        $CacheData,
        [int]$ExpiryHours
    )
    if ($null -eq $CacheData -or -not $CacheData.CacheMetadata -or -not $CacheData.Data) {
        return $false
    }

    # Verify cache belongs to this system configuration
    if ($CacheData.CacheMetadata.HostName -ne $env:COMPUTERNAME) {
        Write-Host "[-] Cache machine mismatch. Invalidating old cache." -ForegroundColor Yellow
        return $false
    }

    $generatedAt = [DateTime]::Parse($CacheData.CacheMetadata.GeneratedAt)
    $expirationTime = $generatedAt.AddHours($ExpiryHours)

    if ((Get-Date) -gt $expirationTime) {
        Write-Host "[-] Cache expired (older than $ExpiryHours hours)." -ForegroundColor Yellow
        return $false
    }

    return $true
}

# -------------------------------------------------------------------------
# MAIN EXECUTION FLOW
# -------------------------------------------------------------------------
function Main {
    $runtimeCache = $null
    $cacheLoaded = $false

    if (-not $ForceRefresh -and (Test-Path -Path $CacheFilePath)) {
        Write-Host "[*] Checking existing cache at: $CacheFilePath" -ForegroundColor Gray
        $runtimeCache = Load-CacheFromFile -Path $CacheFilePath
        
        if (Test-CacheValid -CacheData $runtimeCache -ExpiryHours $CacheExpiryHours) {
            Write-Host "[+] Cache hit! Loading configuration from file to save execution time." -ForegroundColor Green
            $cacheLoaded = $true
        }
    }

    if (-not $cacheLoaded) {
        if ($ForceRefresh) { Write-Host "[*] Force refresh triggered." -ForegroundColor Yellow }

$runtimeCache = Invoke-EnvironmentDetection
Save-CacheToFile -Data $runtimeCache -Path $CacheFilePath
}
# Output formatted, scannable summary report to terminal
Write-Host "`n==== ENVIRONMENT COMPILER & STATUS REPORT ====" -ForegroundColor Cyan
Write-Host "Generated: $($runtimeCache.CacheMetadata.GeneratedAt)" -ForegroundColor Gray
Write-Host "==============================================" -ForegroundColor Cyan
foreach ($category in $runtimeCache.Data.PSObject.Properties.Name) {
Write-Host "n[$category]" -ForegroundColor Magenta foreach ($tool in $runtimeCache.Data.$category.PSObject.Properties.Name) { $item = $runtimeCache.Data.$category.$tool if ($item.Present) { Write-Host "  [✓] $tool " -NoNewline -ForegroundColor Green Write-Host "-> Version: $($item.Version) | Path: $($item.Path)" -ForegroundColor White } else { Write-Host "  [X] $tool " -NoNewline -ForegroundColor Red Write-Host "-> Missing or not in system PATH" -ForegroundColor DarkGray } } } Write-Host "n==============================================" -ForegroundColor Cyan
# Return structured runtime cache object for pipeline ingestion
return $runtimeCache
}

### Key Features Included:
* **Caching Layer**: Saves the JSON output locally. Runs down to milliseconds on subsequent launches if the cache is still valid.
* **Regex-Based Detection**: Rather than just querying if a command exists, it extracts exact version strings for complex dependencies.
* **Resilient**: Captures system changes (e.g., if you copy the cache payload to a different machine, the script auto-invalidates it by comparing hostnames).

If you want to know **how to customize the cache TTL** or **add a specific proprietary compiler** to the list, let me know!