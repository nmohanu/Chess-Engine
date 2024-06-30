#include "util.hpp"

static inline uint8_t get_ls1b_index(uint64_t bitboard)
{
    if (bitboard)
    {
        return __builtin_popcountll((bitboard & -bitboard) - 1);
    }
    else
        return -1;
}

void print_bitboard(uint64_t bitboard)
{
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index
            int square = rank * 8 + file;
            
            // print ranks
            if (!file)
                printf("  %d ", 8 - rank);
            
            // print bit state (either 1 or 0)
            printf(" %d", get_bit_64(bitboard, square) ? 1 : 0);
            
        }
        // print new line every rank
        printf("\n");
    }
    
}
