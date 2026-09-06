#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Find the project root directory relative to this script (two levels up)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

CLANG_FORMAT="clang-format"

# Check if clang-format is installed
if ! command -v "$CLANG_FORMAT" &> /dev/null; then
    echo "Error: '$CLANG_FORMAT' could not be found. Please install it first."
    exit 1
fi

echo "Scanning for source files under: $ROOT_DIR"

# Find files matching the extensions and exclude build directories
# -prune stops find from entering excluded folders, making it much faster
find "$ROOT_DIR" \
    -type d \( -name ".git" -o -name "build" -o -name "Build" -o -name "CMakeFiles" -o -name "vcpkg_installed" \) -prune \
    -o -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" -o -name "*.h" -o -name "*.hh" -o -name "*.hpp" -o -name "*.hxx" \) -print0 | while IFS= read -r -d '' file; do
        
        echo "Formatting: ${file#$ROOT_DIR/}"
        "$CLANG_FORMAT" -i --style=file "$file"
        
    done

echo "Success: Finished formatting your C/C++ project!"


# chmod +x clangformat.sh 