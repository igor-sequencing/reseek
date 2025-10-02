#include <iostream>
#include <cstring>
#include "myutils.h"
#include "parasail.h"

extern parasail_matrix_t parasail_mu_matrix;

int main() {
    std::cout << "Testing NEON/Parasail implementation..." << std::endl;

    // Test sequence alignment
    const char* seq1 = "ABCDEFGHIJKLMNOP";
    const char* seq2 = "ABCDEFGHIJKLMNOP";
    int len1 = strlen(seq1);
    int len2 = strlen(seq2);

    // Create profile
    parasail_profile_t* profile = parasail_profile_create_256_8(seq1, len1, &parasail_mu_matrix);
    if (!profile) {
        std::cerr << "Failed to create profile" << std::endl;
        return 1;
    }

    // Run alignment
    int open = 3;
    int extend = 1;
    parasail_result_t* result = parasail_sw_striped_profile_256_8(profile, seq2, len2, open, extend);

    if (result) {
        std::cout << "Alignment score: " << result->score << std::endl;
        std::cout << "End query: " << result->end_query << std::endl;
        std::cout << "End ref: " << result->end_ref << std::endl;

        if (result->flag & PARASAIL_FLAG_SATURATED) {
            std::cout << "Warning: Result saturated" << std::endl;
        }

        parasail_result_free(result);
    } else {
        std::cerr << "Alignment failed" << std::endl;
        parasail_profile_free(profile);
        return 1;
    }

    parasail_profile_free(profile);

    // Test with different sequences
    const char* seq3 = "ABCDEFGHIJK";
    const char* seq4 = "ABCXYZGHIJK";
    len1 = strlen(seq3);
    len2 = strlen(seq4);

    profile = parasail_profile_create_256_8(seq3, len1, &parasail_mu_matrix);
    result = parasail_sw_striped_profile_256_8(profile, seq4, len2, open, extend);

    if (result) {
        std::cout << "\nTest 2 - Alignment with mismatches:" << std::endl;
        std::cout << "Seq1: " << seq3 << std::endl;
        std::cout << "Seq2: " << seq4 << std::endl;
        std::cout << "Score: " << result->score << std::endl;
        parasail_result_free(result);
    }

    parasail_profile_free(profile);

    std::cout << "\nNEON/Parasail tests completed successfully!" << std::endl;
    return 0;
}