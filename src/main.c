#include <stdio.h>

#include "moves.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>

State state;
PrecomputedAttacks attacks;

uint64_t perft(int depth){
    if (depth == 0) return 1;
    uint64_t nodes = 0;
    MoveList ml;
    ml.count = 0;
    generate_legal_moves(&state, &ml, &attacks);
    for (int i = 0; i < ml.count; i++){
        make_move(&state, &ml.moves[i]);
        nodes += perft(depth - 1);
        unmake_move(&state, &ml.moves[i]);
    }
    return nodes;
}

void sqname(char name[3], int sq){
    name[0] = 'a' + (sq % 8);
    name[1] = '1' + (sq / 8);
    name[2] = 0;
}

void divide(int depth){
    MoveList ml;
    ml.count = 0;
    generate_legal_moves(&state, &ml, &attacks);
    uint64_t total = 0;
    for (int i = 0; i < ml.count; i++){
        make_move(&state, &ml.moves[i]);
        uint64_t n = perft(depth - 1);
        total += n;
        char fr[3]; char to[3];
        sqname(fr, ml.moves[i].from);
        sqname(to, ml.moves[i].to);
        printf("%s->%s: %llu\n", fr, to, n);
        unmake_move(&state, &ml.moves[i]);
    }
    printf("total: %llu\n", total);
}

int main(int argc, char* argv[]){
    board_setup(state.board, state.occupancy);
    precompute_attacks(&attacks);
    printf("%d\n", perft(5));
    return 0;
}