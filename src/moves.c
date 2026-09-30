#include "moves.h"

inline uint8_t popcnt(uint64_t v){
    return __builtin_popcountll(v);
}

inline uint8_t ctz(uint64_t v){
    return __builtin_ctzll(v);
}

uint8_t is_square_attacked(
    State* state, 
    PrecomputedAttacks* attacks, 
    uint8_t square, 
    uint8_t attacking_side)
{
    uint64_t occ = state->occupancy[OCCUPIED_BOTH];
    if (attacking_side == SIDE_WHITE){
        if (attacks->pawns[SIDE_BLACK][square] & state->board[PIECE_WPAWN]) return 1;
        if (attacks->knights[square] & state->board[PIECE_WKNIGHT]) return 1;
        if (attacks->kings[square] & state->board[PIECE_WKING]) return 1;
        if (lookup_bishop_attacks(square, occ) & (state->board[PIECE_WBISHOP] | state->board[PIECE_WQUEEN])) return 1;
        if (lookup_rook_attacks(square, occ) & (state->board[PIECE_WROOK] | state->board[PIECE_WQUEEN])) return 1;
    } else {
        if (attacks->pawns[SIDE_WHITE][square] & state->board[PIECE_BPAWN]) return 1;
        if (attacks->knights[square] & state->board[PIECE_BKNIGHT]) return 1;
        if (attacks->kings[square] & state->board[PIECE_BKING]) return 1;
        if (lookup_bishop_attacks(square, occ) & (state->board[PIECE_BBISHOP] | state->board[PIECE_BQUEEN])) return 1;
        if (lookup_rook_attacks(square, occ) & (state->board[PIECE_BROOK] | state->board[PIECE_BQUEEN])) return 1;
    }
    return 0;
}

void generate_psuedo_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    generate_pawn_moves(state, ml, attacks);
    generate_knight_moves(state, ml, attacks);
    generate_bishop_moves(state, ml);
    generate_rook_moves(state, ml);
    generate_queen_moves(state, ml);
    generate_king_moves(state, ml, attacks);
}

void generate_legal_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    MoveList psuedo;
    psuedo.count = 0;
    generate_psuedo_moves(state, &psuedo, attacks);
    ml->count = 0;
    uint8_t moving_side = state->current_side;
    for (int i = 0; i < psuedo.count; i++){
        make_move(state, &psuedo.moves[i]);
        uint8_t king_piece = (moving_side == SIDE_WHITE) ? PIECE_WKING : PIECE_BKING;
        uint8_t king_square = ctz(state->board[king_piece]);
        uint8_t opponent = moving_side ^ 1;
        if (!is_square_attacked(state, attacks, king_square, opponent)){
            ml->moves[ml->count++] = psuedo.moves[i];
        }
        unmake_move(state, &psuedo.moves[i]);
    }
}

void make_move(State* state, Move* move){
    uint8_t tx = move->to % 8;
    uint8_t ty = move->to / 8;
    uint8_t fx = move->from % 8;
    uint8_t fy = move->from / 8;
    if (move->captured_piece != PIECE_NONE  && !(move->flags & MVFLAG_EN_PASSANT)) {
        board_set_low(&state->board[move->captured_piece], tx, ty);
    } else if (move->flags & MVFLAG_EN_PASSANT) {
        board_set_low(
            &state->board[move->captured_piece],
            tx, 
            state->current_side == SIDE_WHITE ? (ty - 1) : (ty + 1)
        );
    } else if (move->flags & (MVFLAG_CASTLE_Q)){
        int y_coord = state->current_side * 7;
        int piece = state->current_side == SIDE_WHITE ? PIECE_WROOK : PIECE_BROOK;
        board_set_high(
            &state->board[piece],
            3, y_coord
        );
        board_set_low(
            &state->board[piece],
            0, y_coord
        );
    } else if (move->flags & (MVFLAG_CASTLE_K)){
        int y_coord = state->current_side * 7;
        int piece = state->current_side == SIDE_WHITE ? PIECE_WROOK : PIECE_BROOK;
        board_set_high(
            &state->board[piece],
            5, y_coord
        );
        board_set_low(
            &state->board[piece],
            7, y_coord
        );
    }
    if (!(move->flags & MVFLAG_DOUBLE_PUSH)){
        state->en_passant = PIECE_NONE;
    } else {
        state->en_passant = move->to;
    }

    if (move->promote_to != PIECE_NONE){
        board_set_high(&state->board[move->promote_to], tx, ty);
        board_set_low(&state->board[move->piece], fx, fy);
    } else {
        board_set_high(&state->board[move->piece], tx, ty);
        board_set_low(&state->board[move->piece], fx, fy);
    }

    if (move->piece == PIECE_WKING){
        state->castling &= (~(CASTLE_WHITE_K | CASTLE_WHITE_Q));
    } else if (move->piece == PIECE_BKING){
        state->castling &= (~(CASTLE_BLACK_K | CASTLE_BLACK_Q));
    }

    if (move->from == 0){
        state->castling &= (~CASTLE_WHITE_Q);
    } else if (move->from == 7){
        state->castling &= (~CASTLE_WHITE_K);
    } else if (move->from == XY_TO_1D(0, 7)){
        state->castling &= (~CASTLE_BLACK_Q);
    } else if (move->from == XY_TO_1D(7, 7)){
        state->castling &= (~CASTLE_BLACK_K);
    }
    if (move->to == 0){
        state->castling &= (~CASTLE_WHITE_Q);
    } else if (move->to == 7){
        state->castling &= (~CASTLE_WHITE_K);
    } else if (move->to == XY_TO_1D(0, 7)){
        state->castling &= (~CASTLE_BLACK_Q);
    } else if (move->to == XY_TO_1D(7, 7)){
        state->castling &= (~CASTLE_BLACK_K);
    }
    state->current_side ^= 1;

    update_occupancy(state->board, state->occupancy);
}

void unmake_move(State* state, Move* move){
    uint8_t tx = move->to % 8;
    uint8_t ty = move->to / 8;
    uint8_t fx = move->from % 8;
    uint8_t fy = move->from / 8;

    state->current_side ^= 1;

    if (move->promote_to == PIECE_NONE && !(move->flags & (MVFLAG_CASTLE_K | MVFLAG_CASTLE_Q))){
        board_set_low(&state->board[move->piece], tx, ty);
        board_set_high(&state->board[move->piece], fx, fy);
    } else if (move->promote_to != PIECE_NONE) {
        board_set_low(&state->board[move->promote_to], tx, ty);
        board_set_high(&state->board[move->piece], fx, fy);
    } else if (move->flags & (MVFLAG_CASTLE_K)){
        board_set_low(&state->board[move->piece], tx, ty);
        board_set_high(&state->board[move->piece], fx, fy);
        int y_coord = state->current_side * 7;
        int piece = state->current_side == SIDE_WHITE ? PIECE_WROOK : PIECE_BROOK;
        board_set_low(
            &state->board[piece],
            5, y_coord
        );
        board_set_high(
            &state->board[piece],
            7, y_coord
        );
    } else if (move->flags & (MVFLAG_CASTLE_Q)){
        board_set_low(&state->board[move->piece], tx, ty);
        board_set_high(&state->board[move->piece], fx, fy);
        int y_coord = state->current_side * 7;
        int piece = state->current_side == SIDE_WHITE ? PIECE_WROOK : PIECE_BROOK;
        board_set_low(
            &state->board[piece],
            3, y_coord
        );
        board_set_high(
            &state->board[piece],
            0, y_coord
        );
    }

    if (move->flags & MVFLAG_EN_PASSANT){
        board_set_high(
            &state->board[move->captured_piece],
            tx, 
            state->current_side == SIDE_WHITE ? (ty - 1) : (ty + 1)
        );
    } else if (move->captured_piece != PIECE_NONE){
        board_set_high(&state->board[move->captured_piece], tx, ty);
    }

    state->castling = move->prev_castling;
    state->en_passant = move->prev_en_passant;
    update_occupancy(state->board, state->occupancy);
}

uint64_t get_necessary_clear_castle_squares(uint8_t side){
    uint64_t b = 0;
    switch (side){
        case CASTLE_WHITE_K:
            return BOARD_SET_HIGH_1D(
                BOARD_SET_HIGH_1D(b, 6), 5
            );
        case CASTLE_WHITE_Q:
            return BOARD_SET_HIGH_1D(
                BOARD_SET_HIGH_1D(
                    BOARD_SET_HIGH_1D(b, 1),
                    2
                ),
                3
            );
        case CASTLE_BLACK_K:
            board_set_high(&b, 6, 7);
            board_set_high(&b, 5, 7);
            return b;
        case CASTLE_BLACK_Q:
            board_set_high(&b, 1, 7);
            board_set_high(&b, 2, 7);
            board_set_high(&b, 3, 7);
            return b;
        default:
            return (uint64_t)-1;
    }
}

void generate_king_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    uint8_t castle_side = (state->current_side == SIDE_WHITE) ? 
                                                    CASTLE_WHITE_Q :
                                                    CASTLE_BLACK_Q;
    uint8_t piece = (state->current_side == SIDE_WHITE) ?
                                            PIECE_WKING :
                                            PIECE_BKING;
    uint64_t board = state->board[piece];
    while (board){
        uint8_t pos = ctz(board);
        uint64_t possible_attacks = attacks->kings[pos] &
                                        (~state->occupancy[state->current_side]);
        while (possible_attacks){
            int to = ctz(possible_attacks);
            uint8_t captured = PIECE_NONE;
            for (int j = (piece == PIECE_WKING) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            Move m = (Move) {
                .captured_piece = captured,
                .flags = 0,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .to = to
            };
            ml->moves[ml->count++] = m;
            possible_attacks &= possible_attacks - 1;
        }
        if (state->castling & castle_side && 
            !(state->occupancy[OCCUPIED_BOTH] &
                get_necessary_clear_castle_squares(castle_side)) &&
            !is_square_attacked(state, attacks, pos, state->current_side ^ 1)
        ){
            uint8_t x = pos % 8;
            uint8_t y = pos / 8;
            Move m = (Move) {
                .to = XY_TO_1D((x - 2), y),
                .captured_piece = PIECE_NONE,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .flags = MVFLAG_CASTLE_Q,
            };
            ml->moves[ml->count++] = m;
        }
        if (state->castling & (castle_side * 2) &&
            !(state->occupancy[OCCUPIED_BOTH] & 
            get_necessary_clear_castle_squares(castle_side * 2)) &&
            !is_square_attacked(state, attacks, pos, state->current_side ^ 1)
        ){
            uint8_t x = pos % 8;
            uint8_t y = pos / 8;
            Move m = (Move) {
                .to = XY_TO_1D((x + 2), y),
                .captured_piece = PIECE_NONE,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .flags = MVFLAG_CASTLE_K,
            };
            ml->moves[ml->count++] = m;
        }
        board &= board - 1;
    }
}

void generate_queen_moves(State* state, MoveList* ml){
    uint8_t piece = (state->current_side == SIDE_WHITE) ?
                                                    PIECE_WQUEEN :
                                                    PIECE_BQUEEN;
    uint64_t board = state->board[piece];
    while (board){
        int pos = ctz(board);
        uint64_t possible_attacks =
            lookup_queen_attacks(pos, state->occupancy[OCCUPIED_BOTH]) &
            (~state->occupancy[state->current_side]);
        while (possible_attacks){
            uint8_t to = ctz(possible_attacks);
            uint8_t captured = PIECE_NONE;
            for (int j = (piece == PIECE_WQUEEN) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            Move m = (Move) {
                .captured_piece = captured,
                .flags = 0,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .to = to
            };
            ml->moves[ml->count++] = m;
            possible_attacks &= possible_attacks - 1;
        }
        board &= board - 1;
    }
}

void generate_rook_moves(State* state, MoveList* ml){
    uint8_t piece = (state->current_side == SIDE_WHITE) ?
                                                    PIECE_WROOK :
                                                    PIECE_BROOK;
    uint64_t board = state->board[piece];
    while (board){
        int pos = ctz(board);
        uint64_t possible_attacks =
            lookup_rook_attacks(pos, state->occupancy[OCCUPIED_BOTH]) &
            (~state->occupancy[state->current_side]);
        while (possible_attacks){
            uint8_t to = ctz(possible_attacks);
            uint8_t captured = PIECE_NONE;
            for (int j = (piece == PIECE_WROOK) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            Move m = (Move) {
                .captured_piece = captured,
                .flags = 0,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .to = to
            };
            ml->moves[ml->count++] = m;
            possible_attacks &= possible_attacks - 1;
        }
        board &= board - 1;
    }
}

void generate_bishop_moves(State* state, MoveList* ml){
    uint8_t piece = (state->current_side == SIDE_WHITE) ?
                                            PIECE_WBISHOP :
                                            PIECE_BBISHOP;
    uint64_t board = state->board[piece];
    while (board){
        int pos = ctz(board);
        uint64_t possible_attacks = 
            lookup_bishop_attacks(pos, state->occupancy[OCCUPIED_BOTH]) &
            (~state->occupancy[state->current_side]);
        while (possible_attacks){
            uint8_t to = ctz(possible_attacks);
            uint8_t captured =  PIECE_NONE;
            for (int j = (piece == PIECE_WBISHOP) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            Move m = (Move) {
                .captured_piece = captured,
                .flags = 0,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .to = to
            };
            ml->moves[ml->count++] = m;
            possible_attacks &= possible_attacks - 1;
        }
        board &= board - 1;
    }
}

void generate_knight_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    uint8_t piece = (state->current_side == SIDE_WHITE) ?
                                            PIECE_WKNIGHT :
                                            PIECE_BKNIGHT;
    uint64_t board = state->board[piece];
    while (board){
        uint8_t pos = ctz(board);
        uint64_t possible_attacks = attacks->knights[pos] &
                                        (~state->occupancy[state->current_side]);
        while (possible_attacks){
            int to = ctz(possible_attacks);
            uint8_t captured = PIECE_NONE;
            for (int j = (piece == PIECE_WKNIGHT) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            Move m = (Move) {
                .captured_piece = captured,
                .flags = 0,
                .from = pos,
                .piece = piece,
                .prev_castling = state->castling,
                .prev_en_passant = state->en_passant,
                .promote_to = PIECE_NONE,
                .to = to
            };
            ml->moves[ml->count++] = m;
            possible_attacks &= possible_attacks - 1;
        }
        board &= board - 1;
    }
}

void generate_pawn_moves(State* state, MoveList* ml, PrecomputedAttacks* attacks){
    uint8_t piece = (state->current_side == SIDE_WHITE) ? 
                                                        PIECE_WPAWN : 
                                                        PIECE_BPAWN;
    int8_t dy = (state->current_side == SIDE_WHITE) ? 1 : -1;
    uint8_t opposite_occupation = (state->current_side == SIDE_WHITE) ? 
                                                            OCCUPIED_BLACK :
                                                            OCCUPIED_WHITE;
    uint64_t board = state->board[piece];
    while (board){
        uint8_t i = ctz(board);
        uint8_t x = i % 8;
        uint8_t y = i / 8;

        if (!board_get(state->occupancy[OCCUPIED_BOTH], x, y + dy)){
            // the pawn is on the last rank, and thus promoting.
            if (y + dy == 7 || y + dy == 0){
                // essentially iterates over all the PIECE_* defines of the same
                // side that the pawn can actually promote to
                for (int j = piece - 8; j < piece; j += 2){
                    Move m = (Move) {
                        .captured_piece = PIECE_NONE,
                        .flags = 0,
                        .from = i,
                        .to = XY_TO_1D(x, (y + dy)),
                        .piece = piece,
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
                    .piece = piece,
                    .prev_castling = state->castling,
                    .prev_en_passant = state->en_passant,
                    .promote_to = PIECE_NONE
                };
                ml->moves[ml->count++] = m;
            }
        }
        if ((y == 1 && piece == PIECE_WPAWN) || (y == 6 && piece == PIECE_BPAWN)){
            // if there is no piece 2 spaces in front of the pawn
            if (!board_get(state->occupancy[OCCUPIED_BOTH], x, y + (dy * 2)) &&
                !board_get(state->occupancy[OCCUPIED_BOTH], x, y + dy)) {
                Move m = (Move) {
                    .captured_piece = PIECE_NONE,
                    .flags = MVFLAG_DOUBLE_PUSH,
                    .from = XY_TO_1D(x, y),
                    .to = XY_TO_1D(x, (y + (dy * 2))),
                    .piece = piece,
                    .prev_castling = state->castling,
                    .prev_en_passant = state->en_passant,
                    .promote_to = PIECE_NONE,
                };
                ml->moves[ml->count++] = m;
            }
        }
        uint64_t possible_attacks = attacks->pawns[state->current_side][i] &
                                    state->occupancy[opposite_occupation];
        while (possible_attacks){
            int to = ctz(possible_attacks);
            uint8_t captured = PIECE_NONE;
            for (int j = (piece == PIECE_WPAWN) ? 1 : 0; j < 12; j += 2){
                if (state->board[j] & (1ULL << to)){
                    captured = j;
                    break;
                }
            }
            if (y + dy != 7 && y + dy != 0){
                Move m = (Move){
                    .from = i,
                    .captured_piece = captured,
                    .flags = 0,
                    .piece = piece,
                    .prev_castling = state->castling,
                    .prev_en_passant = state->en_passant,
                    .promote_to = PIECE_NONE,
                    .to = to
                };
                ml->moves[ml->count++] = m;
            } else {
                for (int j = piece - 8; j < piece; j += 2){
                    Move m = (Move) {
                        .captured_piece = captured,
                        .flags = 0,
                        .from = i,
                        .to = to,
                        .piece = piece,
                        .prev_castling = state->castling,
                        .prev_en_passant = state->en_passant,
                        .promote_to = j
                    };
                    ml->moves[ml->count++] = m;
                }
            }
            possible_attacks &= possible_attacks - 1;
        }
        
        if (state->en_passant != PIECE_NONE){
                int ex = state->en_passant % 8;
                int ey = state->en_passant / 8;
                if (ey == y && (ex + 1 == x || ex - 1 == x)){
                    Move m = (Move){
                        .from = i,
                        .to = XY_TO_1D(ex, (y + dy)),
                        .captured_piece = (piece == PIECE_WPAWN) ? PIECE_BPAWN : PIECE_WPAWN,
                        .flags = MVFLAG_EN_PASSANT,
                        .piece = piece,
                        .prev_castling = state->castling,
                        .prev_en_passant = state->en_passant,
                        .promote_to = PIECE_NONE,
                    };
                    ml->moves[ml->count++] = m;
                }
            }

        board &= board - 1;
    }
}


void precompute_attacks(PrecomputedAttacks* attacks){
    compute_pawn_attacks(attacks->pawns);
    compute_knight_attacks(attacks->knights);
    compute_king_attacks(attacks->kings);
}