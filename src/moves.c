#include "moves.h"

void generate_psuedo_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    generate_pawn_moves(state, ml, attacks);
    generate_rook_moves(state, ml);
    generate_knight_moves(state, ml, attacks);
    generate_bishop_moves(state, ml);
    generate_queen_moves(state, ml);
    generate_king_moves(state, ml, attacks);
}

void generate_pawn_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    uint8_t side = (state->current_side == SIDE_WHITE) ? PIECE_WPAWN : PIECE_BPAWN;
    int8_t dy = (state->current_side == SIDE_WHITE) ? 1 : -1;
    for (int i = 0; i < 64; i++){
        // if there is no pawn on this square, skip it
        if (!BOARD_GET_1D(state->board[side], i)) continue;
        uint8_t x = i % 8;
        uint8_t y = i / 8;
        // if there is no piece in front of the pawn
        if (!board_get(state->occupancy[OCCUPIED_BOTH], x, y + dy)){
            // the pawn is on the last rank, and thus promoting.
            if (y + dy == 7 || y + dy == 0){
                // essentially iterates over all the PIECE_* defines of the same
                // side that the pawn can actually promote to
                for (int j = side - 8; j < side; j += 2){
                    Move m = (Move) {
                        .captured_piece = PIECE_NONE,
                        .flags = 0,
                        .from = i,
                        .to = XY_TO_1D(x, (y + dy)),
                        .piece = side,
                        .prev_castling = state->castling,
                        .prev_en_passant = state->en_passant,
                        .promote_to = j
                    };
                    ml->moves[ml->count++] = m;
                }
            } else {
                Move m = (Move) {
                    .captured_piece = PIECE_NONE,
                    .flags = 0,
                    .from = i,
                    .to = XY_TO_1D(x, (y + dy)),
                    .piece = side,
                    .prev_castling = state->castling,
                    .prev_en_passant = state->en_passant,
                    .promote_to = PIECE_NONE
                };
                ml->moves[ml->count++] = m;
            }
        }
        if ((y == 1 && side == PIECE_WPAWN) || (y == 6 && side == PIECE_BPAWN)){
            // if there is no piece 2 spaces in front of the pawn
            if (!board_get(state->occupancy[OCCUPIED_BOTH], x, y + (dy * 2))) {
                Move m = (Move) {
                    .captured_piece = PIECE_NONE,
                    .flags = MVFLAG_DOUBLE_PUSH,
                    .from = XY_TO_1D(x, y),
                    .to = XY_TO_1D(x, (y + (dy * 2))),
                    .piece = side,
                    .prev_castling = state->castling,
                    .prev_en_passant = state->en_passant,
                    .promote_to = PIECE_NONE,
                };
                ml->moves[ml->count++] = m;
            }
        }
    }
}

static inline void precompute_attacks(PrecomputedAttacks* attacks){
    compute_pawn_attacks(attacks->pawns);
    compute_knight_attacks(attacks->knights);
    compute_king_attacks(attacks->kings);
}