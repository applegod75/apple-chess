#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define BOARD_WIDTH 8

#define BOARD_GET_1D(_BASE, _POS) ((_BASE & ((uint64_t)1 << _POS)) > 0)

#define XY_TO_1D(_X, _Y) \
(BOARD_WIDTH * _Y + _X)

typedef struct {
	uint64_t mask;
	uint64_t magic;
	uint64_t* table;
	uint64_t shift;
    int table_size;
} DumpableMagicEntry;

uint64_t generate_rook_relevant_mask(uint8_t pos){
    uint8_t x = pos % 8;
    uint8_t y = pos / 8;
    uint64_t rtn = 0;
    for(int nx = x - 1; nx >= 1; nx--) rtn |= 1ULL << (XY_TO_1D(nx, y));
    for(int nx = x + 1; nx <= 6; nx++) rtn |= 1ULL << (XY_TO_1D(nx, y));
    for(int ny = y - 1; ny >= 1; ny--) rtn |= 1ULL << (XY_TO_1D(x, ny));
    for(int ny = y + 1; ny <= 6; ny++) rtn |= 1ULL << (XY_TO_1D(x, ny));
    return rtn;
}

uint64_t generate_rook_attacks(uint8_t pos, uint64_t occupancy){
    uint8_t x = pos % 8;
    uint8_t y = pos / 8;
    uint64_t rtn = 0;
    // south
    for(int ny = y - 1; ny >= 0; ny--){
        int sq = XY_TO_1D(x, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    // north
    for(int ny = y + 1; ny <= 7; ny++){
        int sq = XY_TO_1D(x, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    // west
    for (int nx = x - 1; nx >= 0; nx--){
        int sq = XY_TO_1D(nx, y);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    // east
    for (int nx = x + 1; nx <= 7; nx++){
        int sq = XY_TO_1D(nx, y);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    return rtn;
}

uint64_t generate_bishop_relevant_mask(uint8_t pos){
    uint8_t x = pos % 8;
    uint8_t y = pos / 8;
    uint64_t rtn = 0;
    for (int nx = x + 1, ny = y + 1; nx <= 6 && ny <= 6; nx++, ny++) rtn |= 1ULL << (XY_TO_1D(nx, ny));
    for (int nx = x - 1, ny = y + 1; nx >= 1 && ny <= 6; nx--, ny++) rtn |= 1ULL << (XY_TO_1D(nx, ny));
    for (int ny = y - 1, nx = x - 1; ny >= 1 && nx >= 1; ny--, nx--) rtn |= 1ULL << (XY_TO_1D(nx, ny));
    for (int ny = y - 1, nx = x + 1; ny >= 1 && nx <= 6; ny--, nx++) rtn |= 1ULL << (XY_TO_1D(nx, ny));
    return rtn;
}

uint64_t generate_bishop_attacks(uint8_t pos, uint64_t occupancy){
    uint8_t x = pos % 8;
    uint8_t y = pos / 8;
    uint64_t rtn = 0;
    // north-east
    for(int nx = x + 1, ny = y + 1; nx <= 7 && ny <= 7; nx++, ny++){
        int sq = XY_TO_1D(nx, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    // north-west
    for(int nx = x - 1, ny = y + 1; nx >= 0 && ny <= 7; nx--, ny++){
        int sq = XY_TO_1D(nx, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    // south-east
    for (int nx = x + 1, ny = y - 1; nx <= 7 && ny >= 0; nx++, ny--){
        int sq = XY_TO_1D(nx, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)) {
            break;
        }
    }
    // south-west
    for(int nx = x - 1, ny = y - 1; nx >= 0 && ny >= 0; nx--, ny--){
        int sq = XY_TO_1D(nx, ny);
        rtn |= 1ULL << sq;
        if (BOARD_GET_1D(occupancy, sq)){
            break;
        }
    }
    return rtn;
}

uint64_t RNG_state = 74378294732989241ULL;

uint64_t randu64(){
    RNG_state ^= RNG_state << 13;
    RNG_state ^= RNG_state >> 7;
    RNG_state ^= RNG_state << 17;
    return RNG_state;
}

uint64_t magic_candidate(){
    return randu64() & randu64() & randu64();
}

static inline int popcnt(uint64_t v){
    return __builtin_popcountll(v);
}

// this generates all of the possible occupancies and then computes
// the possible attacks for those occupancies to create pairs of
// an occupancy and then an attack.
int generate_occupancy_attack_pairs(
    uint8_t pos, 
    uint64_t relevant_squares_mask,
    uint64_t (*ray_fn)(uint8_t pos, uint64_t occupancy),
    uint64_t* occupancies,
    uint64_t* attacks
){
    int i = 0;
    uint64_t subset = 0;
    do {
        occupancies[i] = subset;
        attacks[i] = ray_fn(pos, subset);
        i++;
        // "iterate" over all possible orderings of the high bits in relevant_squares_mask.
        subset = (subset - relevant_squares_mask) & relevant_squares_mask;
    } while (subset != 0);
    return i;
}

uint64_t find_magic_for_square(uint64_t mask, uint64_t* occupancies, uint64_t* attacks, int table_size, uint64_t* used_table){
    int n = popcnt(mask);
    int shift = 64 - n;
    uint64_t* tried_table = malloc(sizeof(uint64_t) * table_size);
    uint8_t* filled = malloc(sizeof(uint8_t) * table_size);
    while (1){
        uint64_t candidate = magic_candidate();
        memset(filled, 0, table_size * sizeof(uint8_t));
        int success = 1;
        for (int i = 0; i < table_size; i++){
            uint64_t index = (occupancies[i] * candidate) >> shift;
            if (!filled[index]){
                filled[index] = 1;
                tried_table[index] = attacks[i];
            } else if (tried_table[index] != attacks[i]){
                success = 0;
                break;
            }
        }
        if (success){
            memcpy(used_table, tried_table, sizeof(uint64_t) * table_size);
            free(tried_table);
            free(filled);
            return candidate;
        }
    }
}

void write_magicdump_header(FILE* f){
    fprintf(f, "// this file was automatically generated by apple-chess.\n");
    fprintf(f, "#ifndef APPLE_CHESS_AUTOGENERATED_MAGICS_H_\n");
    fprintf(f, "#define APPLE_CHESS_AUTOGENERATED_MAGICS_H_\n\n");
    fprintf(f, "#include <stdint.h>\n\n");
    fprintf(f, "typedef struct {\n");
    fprintf(f, "\tuint64_t mask;\n");
    fprintf(f, "\tuint64_t magic;\n");
    fprintf(f, "\tuint64_t* table;\n");
    fprintf(f, "\tuint64_t shift;\n");
    fprintf(f, "} MagicEntry;\n\n");
    fprintf(f, "extern MagicEntry rook_magics[64];\n");
    fprintf(f, "extern MagicEntry bishop_magics[64];\n\n");
    fprintf(f, "#endif");
}

void generate_bishop_magic_entries(DumpableMagicEntry bishops[64]){
        for(int ny = 0; ny < 8; ny++){
        for (int nx = 0; nx < 8; nx++){
            uint64_t mask = generate_bishop_relevant_mask(XY_TO_1D(nx, ny));
            int n = __builtin_popcountll(mask);
            int table_size = 1 << n;
            uint64_t occupancies[4096];
            uint64_t attacks[4096];
            uint64_t* used = malloc(table_size * sizeof(uint64_t));
            int count = generate_occupancy_attack_pairs(XY_TO_1D(nx, ny), generate_bishop_relevant_mask(XY_TO_1D(nx, ny)), generate_bishop_attacks, occupancies, attacks);
            assert(count == table_size);
            uint64_t magic = find_magic_for_square(mask, occupancies, attacks, table_size, used);
            bishops[XY_TO_1D(nx, ny)] = (DumpableMagicEntry){
                .mask = mask,
                .magic = magic,
                .table = used,
                .shift = 64 - n,
                .table_size = table_size
            };
        }
    }
}

void generate_rook_magic_entries(DumpableMagicEntry rooks[64]){
        for(int ny = 0; ny < 8; ny++){
        for (int nx = 0; nx < 8; nx++){
            uint64_t mask = generate_rook_relevant_mask(XY_TO_1D(nx, ny));
            int n = __builtin_popcountll(mask);
            int table_size = 1 << n;
            uint64_t occupancies[4096];
            uint64_t attacks[4096];
            uint64_t* used = malloc(table_size * sizeof(uint64_t));
            int count = generate_occupancy_attack_pairs(XY_TO_1D(nx, ny), generate_rook_relevant_mask(XY_TO_1D(nx, ny)), generate_rook_attacks, occupancies, attacks);
            assert(count == table_size);
            uint64_t magic = find_magic_for_square(mask, occupancies, attacks, table_size, used);
            rooks[XY_TO_1D(nx, ny)] = (DumpableMagicEntry){
                .mask = mask,
                .magic = magic,
                .table = used,
                .shift = 64 - n,
                .table_size = table_size
            };
        }
    }
}

int main(int argc, char** argv){
    if (argc < 2){
        printf("usage: magic_generator <target filename>\n");
        return 1;
    }

    size_t header_len = strlen(argv[1]) + 3;
    char* header_target = malloc(header_len);
    strcpy(header_target, argv[1]);
    header_target = strcat(header_target, ".h");
    FILE* header = fopen(header_target, "w");
    write_magicdump_header(header); 
    fclose(header);

    DumpableMagicEntry rooks[64];
    DumpableMagicEntry bishops[64];
    generate_bishop_magic_entries(bishops);
    generate_rook_magic_entries(rooks);


    size_t src_len = strlen(argv[1]) + 3;
    char* src_target = malloc(src_len);
    strcpy(src_target, argv[1]);
    src_target = strcat(src_target, ".c");
    FILE* f = fopen(src_target, "w");


    if (strlen(header_target) <= 0 || tolower(header_target[0]) != 's') goto write;
    if (strlen(header_target) <= 1 || tolower(header_target[1]) != 'r') goto write;
    if (strlen(header_target) <= 2 || tolower(header_target[2]) != 'c') goto write;
    if (strlen(header_target) <= 3 || (header_target[3] != '/' && header_target[3] != '\\')) goto write;

    // skip over the src/ prefix if it exists
    header_target = header_target + (4 * sizeof(char));

write:
    fprintf(f, "// this file was automatically generated by apple-chess.\n");
    fprintf(f, "// all values in this file are required for the engine to run\n");
    fprintf(f, "// and changing any of these values may cause crashes\n\n");
    fprintf(f, "#include \"%s\"\n", header_target);

    for(int i = 0; i < 64; i++){
        fprintf(f, "static uint64_t rook_table_sq%d[%d] = {", i, rooks[i].table_size);
        for (int j = 0; j < rooks[i].table_size; j++){
            if (j % 5 == 0) fprintf(f, "\n\t");
            fprintf(f, "0x%016llXULL", rooks[i].table[j]);
            if (j + 1 == rooks[i].table_size){
                fprintf(f, "\n};\n");
            } else {
                fprintf(f, ", ");
            }
        }
        fprintf(f, "static uint64_t bishop_table_sq%d[%d] = {", i, bishops[i].table_size);
        for (int j = 0; j < bishops[i].table_size; j++){
            if (j % 5 == 0) fprintf(f, "\n\t");
            fprintf(f, "0x%016llXULL", bishops[i].table[j]);
            if (j + 1 == bishops[i].table_size){
                fprintf(f, "\n};\n");
            } else {
                fprintf(f, ", ");
            }
        }
        free(rooks[i].table);
        free(bishops[i].table);
    }

    fprintf(f, "MagicEntry rook_magics[64] = {\n");
    for (int i = 0; i < 64; i++){
        fprintf(f, "\t[%d] = { .mask = 0x%016llXULL, .magic = 0x%016llXULL, ", i, rooks[i].mask, rooks[i].magic);
        fprintf(f, ".table = rook_table_sq%d, .shift = %d},\n", i, rooks[i].shift);
    }
    fprintf(f, "};\n");
    fprintf(f, "MagicEntry bishop_magics[64] = {\n");
    for(int i = 0; i < 64; i++){
        fprintf(f, "\t[%d] = { .mask = 0x%016llXULL, .magic = 0x%016llXULL, ", i, bishops[i].mask, bishops[i].magic);
        fprintf(f, ".table = rook_table_sq%d, .shift = %d},\n", i, bishops[i].shift);
    }
    fprintf(f, "};\n");
    fclose(f);

    return 0;
}