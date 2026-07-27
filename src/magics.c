#include "magics.h"
#include "attacks.h"

uint64_t RNG_state = 74378294732989241ULL;

uint64_t randu64(){
    RNG_state ^= RNG_state << 13;
    RNG_state ^= RNG_state >> 7;
    RNG_state ^= RNG_state << 17;
    return RNG_state;
}

void validate_magic_lookups() {
    for (int sq = 0; sq < 64; sq++) {
        for (int trial = 0; trial < 10000; trial++) {
            uint64_t occ = randu64() & randu64();

            uint64_t expected_rook = generate_rook_attacks(sq, occ);
            uint64_t actual_rook = lookup_rook_attacks(sq, occ);
            assert(expected_rook == actual_rook);

            uint64_t expected_bishop = generate_bishop_attacks(sq, occ);
            uint64_t actual_bishop = lookup_bishop_attacks(sq, occ);
            assert(expected_bishop == actual_bishop);
        }
    }
    printf("all magic lookups validated correctly\n");
}