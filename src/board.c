#include "board.h"

uint64_t board_flip(uint64_t base, uint8_t x, uint8_t y){
    uint64_t shiftAmount = XY_TO_1D(x, y);
    return base ^ ((uint64_t)1 << shiftAmount);
}

int board_get(uint64_t base, uint8_t x, uint8_t y){
    uint64_t position = XY_TO_1D(x, y);
    return (base & ((uint64_t)1 << position)) > 0;
}

uint64_t board_set_high(uint64_t base, uint8_t x, uint8_t y){
    uint64_t shiftAmount = XY_TO_1D(x, y);
    return base | ((uint64_t)1 << shiftAmount);
}

uint64_t board_set_low(uint64_t base, uint8_t x, uint8_t y){
    uint64_t position = XY_TO_1D(x, y);
    if (BOARD_GET_1D(base, position)) return BOARD_FLIP_1D(base, position);
    return base;
}

void board_setup(uint64_t pos_boards[12], uint64_t occupancy_boards[3]){
    memset(pos_boards, 0, 12 * sizeof(uint64_t));
    // kings
    pos_boards[PIECE_WKING] = board_set_high(pos_boards[PIECE_WKING], 4, 0);
    pos_boards[PIECE_BKING] = board_set_high(pos_boards[PIECE_BKING], 4, 7);
    // queens
    pos_boards[PIECE_WQUEEN] = board_set_high(pos_boards[PIECE_WQUEEN], 3, 0);
    pos_boards[PIECE_BQUEEN] = board_set_high(pos_boards[PIECE_BQUEEN], 3, 7);
    // rooks
    pos_boards[PIECE_WROOK] = board_set_high(pos_boards[PIECE_WROOK], 0, 0);
    pos_boards[PIECE_WROOK] = board_set_high(pos_boards[PIECE_WROOK], 7, 0);
    pos_boards[PIECE_BROOK] = board_set_high(pos_boards[PIECE_BROOK], 0, 7);
    pos_boards[PIECE_BROOK] = board_set_high(pos_boards[PIECE_BROOK], 7, 7);
    // bishops
    pos_boards[PIECE_WBISHOP] = board_set_high(pos_boards[PIECE_WBISHOP], 2, 0);
    pos_boards[PIECE_WBISHOP] = board_set_high(pos_boards[PIECE_WBISHOP], 5, 0);
    pos_boards[PIECE_BBISHOP] = board_set_high(pos_boards[PIECE_BBISHOP], 2, 7);
    pos_boards[PIECE_BBISHOP] = board_set_high(pos_boards[PIECE_BBISHOP], 5, 7);
    // knights
    pos_boards[PIECE_WKNIGHT] = board_set_high(pos_boards[PIECE_WKNIGHT], 1, 0);
    pos_boards[PIECE_WKNIGHT] = board_set_high(pos_boards[PIECE_WKNIGHT], 6, 0);
    pos_boards[PIECE_BKNIGHT] = board_set_high(pos_boards[PIECE_BKNIGHT], 1, 7);
    pos_boards[PIECE_BKNIGHT] = board_set_high(pos_boards[PIECE_BKNIGHT], 6, 7);
    // white pawns
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 0, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 1, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 2, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 3, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 4, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 5, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 6, 1);
    pos_boards[PIECE_WPAWN] = board_set_high(pos_boards[PIECE_WPAWN], 7, 1);
    // black pawns
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 0, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 1, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 2, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 3, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 4, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 5, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 6, 6);
    pos_boards[PIECE_BPAWN] = board_set_high(pos_boards[PIECE_BPAWN], 7, 6);
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

void set_piece(Position* pos, uint8_t p, int piece){
    if (!BOARD_GET_1D(pos->position_boards[piece], p)){
        pos->position_boards[piece] = BOARD_FLIP_1D(pos->position_boards[piece], p);
    }
}

void remove_piece(Position* pos, uint8_t p, int piece){
    if (BOARD_GET_1D(pos->position_boards[piece], p)){
        pos->position_boards[piece] = BOARD_FLIP_1D(pos->position_boards[piece], p);
    }
}

void move_piece(Position* pos, Move move){
    if (move.captured != PIECE_NONE){
        remove_piece(pos, move.to, move.captured);
    }
    remove_piece(pos, move.from, move.piece);
    set_piece(pos, move.to, move.piece);
}

void update_occupancy(Position* pos){
        pos->occupied[OCCUPIED_WHITE] = (
        pos->position_boards[PIECE_WPAWN]   |
        pos->position_boards[PIECE_WKNIGHT] |
        pos->position_boards[PIECE_WBISHOP] |
        pos->position_boards[PIECE_WROOK]   |
        pos->position_boards[PIECE_WQUEEN]  |
        pos->position_boards[PIECE_WKING]
    );
    pos->occupied[OCCUPIED_BLACK] = (
        pos->position_boards[PIECE_BPAWN]   |
        pos->position_boards[PIECE_BKNIGHT] |
        pos->position_boards[PIECE_BBISHOP] |
        pos->position_boards[PIECE_BROOK]   |
        pos->position_boards[PIECE_BQUEEN]  |
        pos->position_boards[PIECE_BKING]
    );
    pos->occupied[OCCUPIED_BOTH] = (
        pos->occupied[OCCUPIED_BLACK] | pos->occupied[OCCUPIED_WHITE]
    );
}

void make_move(Position* pos, Move mov, Undo* undo){
    undo->captured = mov.captured;
    undo->castling = pos->castling;
    undo->en_passant = pos->en_passant;
    undo->hash = pos->hash;
    move_piece(pos, mov);
    pos->side ^= 1;
    update_occupancy(pos);
}

void unmake_move(Position* pos, Move mov, Undo* undo){
    pos->side ^= 1;
    remove_piece(pos, mov.to, mov.piece);
    if (undo->captured != PIECE_NONE){
        set_piece(pos, mov.to, undo->captured);
    }
    set_piece(pos, mov.from, mov.piece);
    pos->castling = undo->castling;
    pos->en_passant = undo->en_passant;
    pos->hash = undo->hash;
    update_occupancy(pos);
}