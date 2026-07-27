#include <stdio.h>

#include "attacks.h"
#include "magics.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

Position pos;

void print_board(uint64_t board){
    for(int y = 0; y < BOARD_HEIGHT; y++){
        for(int x = 0; x < BOARD_WIDTH; x++){
            printf("%d ", board_get(board, x, y));
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]){
    validate_magic_lookups();

    return 0;
}