#!/bin/bash

# Build script with architecture auto-detection
# Uses native Makefile instead of vcxproj_make

set -e  # Exit on error

echo "=== Building reseek with native Makefile ==="
echo

# Show detected configuration
echo "Platform: $(uname -s)"
echo "Architecture: $(uname -m)"
echo "Compiler: ${CC:-gcc}/${CXX:-g++}"
echo "CPU cores: $(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"
echo

# Clean and build using the generated makefile
make -f Makefile.generated clean || true
make -f Makefile.generated -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo
echo "=== Build complete ==="
ls -lh ../bin/reseek