#ifndef APPLE_CHESS_MAGICS_H_
#define APPLE_CHESS_MAGICS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

int generate_occupancy_attack_pairs(
    uint8_t pos, 
    uint64_t relevant_squares_mask,
    uint64_t (*ray_fn)(uint8_t pos, uint64_t occupancy),
    uint64_t* occupancies,
    uint64_t* attacks
);

typedef struct {
	uint64_t mask;
	uint64_t magic;
	uint64_t* table;
	uint64_t shift;
    int table_size;
} DumpableMagicEntry;

uint64_t find_magic_for_square(uint64_t mask, uint64_t* occupancies, uint64_t* attacs, int table_size, uint64_t* used_table);

void write_magicdump_header(FILE* f);
void generate_bishop_magic_entries(DumpableMagicEntry bishops[64]);
void generate_rook_magic_entries(DumpableMagicEntry rooks[64]);

#endif