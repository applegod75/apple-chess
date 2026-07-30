#ifndef APPLE_CHESS_ATTACKS_H_
#define APPLE_CHESS_ATTACKS_H_

#include "board.h"

void compute_knight_attacks(uint64_t knight_attacks[64]);
void compute_king_attacks(uint64_t king_attacks[64]);
void compute_pawn_attacks(uint64_t pawn_attacks[2][64]);

uint64_t generate_knight_attack(uint8_t pos);
uint64_t generate_king_attack(uint8_t pos);
uint64_t generate_pawn_attack(uint8_t pos, uint8_t side);

uint64_t lookup_rook_attacks(uint8_t square, uint64_t occupancy);
uint64_t lookup_bishop_attacks(uint8_t square, uint64_t occupancy);
uint64_t lookup_queen_attacks(uint8_t square, uint64_t occupancy);

#endif