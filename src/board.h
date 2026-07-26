
#ifndef APPLE_CHESS_BOARD_H_
#define APPLE_CHESS_BOARD_H_

#include <stdint.h>
#include <memory.h>

#define PIECE_WKING 0
#define PIECE_BKING 1
#define PIECE_WQUEEN 2
#define PIECE_BQUEEN 3
#define PIECE_WROOK 4
#define PIECE_BROOK 5
#define PIECE_WBISHOP 6
#define PIECE_BBISHOP 7
#define PIECE_WKNIGHT 8
#define PIECE_BKNIGHT 9
#define PIECE_WPAWN 10
#define PIECE_BPAWN 11

#define PIECE_NONE 255

#define OCCUPIED_WHITE 0
#define OCCUPIED_BLACK 1
#define OCCUPIED_BOTH 2

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define SIDE_WHITE 0
#define SIDE_BLACK 1

#define CASTLING_RIGHT_WQ 0b1000
#define CASTLING_RIGHT_WK 0b0100
#define CASTLING_RIGHT_BQ 0b0010
#define CASTLING_RIGHT_BK 0b0001

#define MOVFLAG_CAPTURE 0b1000
#define MOVFLAG_QUIET 0x0

#define XY_TO_1D(_X, _Y) \
(BOARD_WIDTH * _Y + _X)

#define BOARD_GET_1D(_BASE, _POS) ((_BASE & ((uint64_t)1 << _POS)) > 0)
#define BOARD_FLIP_1D(_BASE, _POS) (_BASE ^ ((uint64_t)1 << _POS))
#define BOARD_SET_HIGH_1D(_BASE, _POS) (_BASE | ((uint64_t)1 << _POS))

uint64_t board_flip(uint64_t base, uint8_t x, uint8_t y);
uint64_t board_set_high(uint64_t base, uint8_t x, uint8_t y);
uint64_t board_set_low(uint64_t base, uint8_t x, uint8_t y);
int board_get(uint64_t base, uint8_t x, uint8_t y);
void board_setup(uint64_t pos_boards[12], uint64_t occupancy_boards[3]);

#pragma pack(push, 1)
typedef struct {
    uint64_t position_boards[12];
    uint64_t occupied[3];
    int side;
    uint8_t castling;
    int en_passant;
    uint64_t hash;
} Position;

typedef struct {
    uint8_t from;
    uint8_t to;

    uint8_t piece;
    uint8_t captured;
} Move;

typedef struct {
    uint8_t captured;
    uint8_t castling;
    int en_passant;
    uint64_t hash;
} Undo;
#pragma pack(pop)

void set_piece(Position* pos, uint8_t p, int piece);
void remove_piece(Position* pos, uint8_t p, int piece);
void move_piece(Position* pos, Move mov);
void position_setup(Position* pos);
void make_move(Position* pos, Move mov, Undo* undo);
void unmake_move(Position* pos, Move mov, Undo* undo);
void update_occupancy(Position* pos);

#endif