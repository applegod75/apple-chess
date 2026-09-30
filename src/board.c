#include "board.h"

uint64_t board_flip(uint64_t base, uint8_t x, uint8_t y){
    uint64_t pos = XY_TO_1D(x, y);
    return base ^ ((uint64_t)1 << pos);
}

uint8_t board_get(uint64_t base, uint8_t x, uint8_t y){
    uint64_t position = XY_TO_1D(x, y);
    return (base & ((uint64_t)1 << position)) > 0;
}

void board_set_high(uint64_t* base, uint8_t x, uint8_t y){
    uint64_t pos = XY_TO_1D(x, y);
    *base = BOARD_SET_HIGH_1D(*base, pos);
}

void board_set_low(uint64_t* base, uint8_t x, uint8_t y){
    uint64_t position = XY_TO_1D(x, y);
    *base = ((*base) & (~(1ULL << position)));
}

void board_setup(uint64_t pos_boards[12], uint64_t occupancy_boards[3]){
    memset(pos_boards, 0, 12 * sizeof(uint64_t));
    // kings
    board_set_high(&pos_boards[PIECE_WKING], 4, 0);
    board_set_high(&pos_boards[PIECE_BKING], 4, 7);
    // queens
    board_set_high(&pos_boards[PIECE_WQUEEN], 3, 0);
    board_set_high(&pos_boards[PIECE_BQUEEN], 3, 7);
    // rooks
    board_set_high(&pos_boards[PIECE_WROOK], 0, 0);
    board_set_high(&pos_boards[PIECE_WROOK], 7, 0);
    board_set_high(&pos_boards[PIECE_BROOK], 0, 7);
    board_set_high(&pos_boards[PIECE_BROOK], 7, 7);
    // bishops
    board_set_high(&pos_boards[PIECE_WBISHOP], 2, 0);
    board_set_high(&pos_boards[PIECE_WBISHOP], 5, 0);
    board_set_high(&pos_boards[PIECE_BBISHOP], 2, 7);
    board_set_high(&pos_boards[PIECE_BBISHOP], 5, 7);
    // knights
    board_set_high(&pos_boards[PIECE_WKNIGHT], 1, 0);
    board_set_high(&pos_boards[PIECE_WKNIGHT], 6, 0);
    board_set_high(&pos_boards[PIECE_BKNIGHT], 1, 7);
    board_set_high(&pos_boards[PIECE_BKNIGHT], 6, 7);
    // white pawns
    board_set_high(&pos_boards[PIECE_WPAWN], 0, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 1, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 2, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 3, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 4, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 5, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 6, 1);
    board_set_high(&pos_boards[PIECE_WPAWN], 7, 1);
    // black pawns
    board_set_high(&pos_boards[PIECE_BPAWN], 0, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 1, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 2, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 3, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 4, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 5, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 6, 6);
    board_set_high(&pos_boards[PIECE_BPAWN], 7, 6);
    // occupancy
    occupancy_boards[OCCUPIED_WHITE] = (
        pos_boards[PIECE_WPAWN]   |
        pos_boards[PIECE_WKNIGHT] |
        pos_boards[PIECE_WBISHOP] |
        pos_boards[PIECE_WROOK]   |
        pos_boards[PIECE_WQUEEN]  |
        pos_boards[PIECE_WKING]
    );
    occupancy_boards[OCCUPIED_BLACK] = (
        pos_boards[PIECE_BPAWN]   |
        pos_boards[PIECE_BKNIGHT] |
        pos_boards[PIECE_BBISHOP] |
        pos_boards[PIECE_BROOK]   |
        pos_boards[PIECE_BQUEEN]  |
        pos_boards[PIECE_BKING]
    );
    occupancy_boards[OCCUPIED_BOTH] = (
        occupancy_boards[OCCUPIED_BLACK] | occupancy_boards[OCCUPIED_WHITE]
    );
}

void update_occupancy(uint64_t pos[12], uint64_t occupancies[3]){
    occupancies[OCCUPIED_WHITE] = (
        pos[PIECE_WPAWN] |
        pos[PIECE_WKNIGHT] |
        pos[PIECE_WBISHOP] |
        pos[PIECE_WROOK] |
        pos[PIECE_WQUEEN] |
        pos[PIECE_WKING]
    );

    occupancies[OCCUPIED_BLACK] = (
        pos[PIECE_BPAWN] |
        pos[PIECE_BKNIGHT] |
        pos[PIECE_BBISHOP] |
        pos[PIECE_BROOK] |
        pos[PIECE_BQUEEN] |
        pos[PIECE_BKING]
    );

    occupancies[OCCUPIED_BOTH] = 
        occupancies[OCCUPIED_BLACK] |
        occupancies[OCCUPIED_WHITE];
}