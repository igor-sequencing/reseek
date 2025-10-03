# Building Reseek

## Architecture Support

This project now supports both x86-64 (with AVX2) and ARM64 (with NEON) architectures. The build system automatically detects your architecture and uses the appropriate SIMD instructions.

### Supported Architectures:
- **x86-64**: Uses AVX2 instructions for SIMD operations
- **ARM64/AArch64**: Uses NEON instructions (Apple M1-M4, ARM servers)

## Build Options

### Option 1: Using Make (Recommended)
```bash
cd src
make
```

For debug build:
```bash
make debug
```

To see configuration:
```bash
make info
```

### Option 2: Using CMake
```bash
cd src
mkdir build
cd build
cmake ..
make -j
```

### Option 3: Using the native build script
```bash
cd src
./make_native.bash
```

### Option 4: Using the original build system (if vcxproj_make is available)
```bash
cd src
./make.bash
```

## Architecture Detection

The build system automatically detects your architecture:

- On **macOS ARM64** (M1-M4): Uses Clang++ with NEON instructions
- On **Linux ARM64**: Uses g++ with NEON instructions
- On **x86-64**: Uses appropriate compiler with AVX2 instructions

## Compiler Flags

### ARM64 (NEON)
- `-march=armv8-a+simd`: Enable ARM SIMD/NEON instructions
- `-D__ARM_NEON`: Define macro for conditional compilation

### x86-64 (AVX2)
- `-mavx2`: Enable AVX2 instructions
- `-mfma`: Enable FMA (Fused Multiply-Add) instructions

## Dependencies

- C++11 compatible compiler
- zlib
- pthread

## Output

The compiled binary will be placed in `../bin/reseek`

## Troubleshooting

If you encounter build issues:

1. Check your architecture: `uname -m`
2. Verify compiler version: `g++ --version` or `clang++ --version`
3. For verbose build: `make VERBOSE=1`
4. Clean build: `make clean && make`

## Performance Notes

The SIMD optimizations provide significant performance improvements for sequence alignment operations. Both AVX2 (x86-64) and NEON (ARM64) implementations provide similar performance benefits on their respective architectures.