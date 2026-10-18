/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

<#
.SYNOPSIS
    Build Environment & Dependency Checker for Windows/PowerShell.
#>

Global:AnyFailed = false

function Write-Success (\$Message) {
    Write-Host "[✓] \$Message" -ForegroundColor Green
}

function Write-Failure (\$Message) {
    Write-Host "[X] \$Message" -ForegroundColor Red
    Global:AnyFailed = true
}

function Check-Command (CommandName, MinimumVersion) {
    Path = Get-Command CommandName -ErrorAction SilentlyContinue
    if (-not \$Path) {
        Write-Failure "\$CommandName is NOT installed or not in PATH."
        return
    }

    if (\$MinimumVersion) {
        # Attempt to extract version number dynamically
        if (\$CommandName -eq "node") {
            \$RawVersion = (node -v) -replace 'v', ''
        } else {
            \(RawVersion = &\)CommandName --version 2>&1 | Out-String
            if (\$RawVersion -match '(\d+\.\d+\.\d+)') { RawVersion = Matches[1] }
        }

        # Validate semantic versioning
        if ([version]\(RawVersion -ge [version]\)MinimumVersion) {
            Write-Success "\$CommandName version RawVersion meets requirement (>= MinimumVersion)."
        } else {
            Write-Failure "CommandName version RawVersion is too old. Required: >= \$MinimumVersion."
        }
    } else {
        Write-Success "\$CommandName is installed."
    }
}

function Check-EnvVar (\$VarName) {
    if ([string]::IsNullOrEmpty([Environment]::GetEnvironmentVariable(\$VarName))) {
        Write-Failure "Environment variable '\$VarName' is NOT set."
    } else {
        \(Value = [Environment]::GetEnvironmentVariable(\)VarName)
        Write-Success "Environment variable 'VarName' is set to: Value"
    }
}

# --- RUN CHECKS ---
Write-Host "=== Starting Build Environment Audit ===" -ForegroundColor Cyan

# 1. Check Tool Dependencies & Versions
Check-Command "git" "2.30.0"
Check-Command "node" "18.0.0"
Check-Command "docker"

# 2. Check Required Environment Variables
Check-EnvVar "NODE_ENV"
Check-EnvVar "JAVA_HOME"

# --- FINAL ASSESSMENT ---
if (\$Global:AnyFailed) {
    Write-Host "`n[!] Environment check FAILED. Please fix the missing dependencies above." -ForegroundColor Red
    Exit 1
} else {
    Write-Host "`n[✓] Environment check PASSED. Ready to build!" -ForegroundColor Green
    Exit 0
}
