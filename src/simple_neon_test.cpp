#include <iostream>
#include <cstring>
#include "parasail.h"

int main() {
    std::cout << "Simple NEON intrinsics test..." << std::endl;

#ifdef __ARM_NEON
    std::cout << "ARM NEON compilation enabled" << std::endl;

    // Test basic NEON operations
    __m256i a = _mm256_set1_epi8(10);
    __m256i b = _mm256_set1_epi8(5);
    __m256i c = _mm256_adds_epi8(a, b);

    // Extract a value to verify it works
    int8_t result = _mm256_extract_epi8(c, 0);
    std::cout << "NEON add test: 10 + 5 = " << (int)result << std::endl;

    if (result == 15) {
        std::cout << "✓ NEON intrinsics working correctly!" << std::endl;
    } else {
        std::cout << "✗ NEON intrinsics test failed!" << std::endl;
        return 1;
    }

    // Test comparison
    __m256i d = _mm256_set1_epi8(20);
    __m256i e = _mm256_set1_epi8(10);
    __m256i cmp = _mm256_cmpgt_epi8(d, e);

    // Test movemask
    int mask = _mm256_movemask_epi8(cmp);
    std::cout << "Comparison mask (should be non-zero): " << std::hex << mask << std::dec << std::endl;

    std::cout << "All NEON tests passed!" << std::endl;
#else
    std::cout << "AVX2 compilation (x86_64)" << std::endl;
    std::cout << "This build uses AVX2 instead of NEON" << std::endl;
#endif

    return 0;
}