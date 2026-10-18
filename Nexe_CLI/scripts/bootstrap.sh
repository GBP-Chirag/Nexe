/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

#!/usr/bin/env bash

# ==============================================================================
# Build Environment & Dependency Checker for Linux / macOS / Git Bash.
# ==============================================================================

# Text formatting colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0;35m' # No Color
NC='\033[0m'

ANY_FAILED=0

log_success() {
    echo -e "${GREEN}[✓] $1${NC}"
}

log_failure() {
    echo -e "${RED}[X] $1${NC}"
    ANY_FAILED=1
}

check_command() {
    local cmd_name=$1
    local min_version=$2

    if ! command -v "$cmd_name" &> /dev/null; then
        log_failure "$cmd_name is NOT installed or not in PATH."
        return
    fi

    if [ -n "$min_version" ]; then
        # Parse version strings safely across platforms
        local raw_version
        if [ "$cmd_name" = "node" ]; then
            raw_version=$(node -v | tr -d 'v')
        else
            raw_version=$($cmd_name --version 2>&1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -n1)
        fi

        # Simple semantic version check using sort -V
        if [ "$(printf '%s\n%s' "$min_version" "$raw_version" | sort -V | head -n1)" = "$min_version" ]; then
            log_success "$cmd_name version $raw_version meets requirement (>= $min_version)."
        fi
    else
        log_success "$cmd_name is installed."
    fi
}

check_env_var() {
    local var_name=$1
    if [ -z "${!var_name}" ]; then
        log_failure "Environment variable '$var_name' is NOT set."
    fi
}

# --- RUN CHECKS ---
echo -e "${CYAN}=== Starting Build Environment Audit ===${NC}"

# 1. Check Tool Dependencies & Versions
check_command "git" "2.30.0"
check_command "node" "18.0.0"
check_command "docker"

# 2. Check Required Environment Variables
check_env_var "NODE_ENV"
check_env_var "JAVA_HOME"

# --- FINAL ASSESSMENT ---
if [ $ANY_FAILED -ne 0 ]; then
    echo -e "\n${RED}[!] Environment check FAILED. Please fix the missing dependencies above.${NC}"
    exit 1
else
    echo -e "\n${GREEN}[✓] Environment check PASSED. Ready to build!${NC}"
    exit 0
fi
