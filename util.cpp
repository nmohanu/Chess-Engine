#include "util.hpp"

// Bishop moving logic.
uint64_t make_bishop_mask(uint8_t square, uint64_t occupation) 
{
    uint64_t move_board = 0b0;

    int square_copy = square;
    uint64_t bit_mask = 1ULL << (63-square);
    // Continue until another piece is found. Repeat for each direction.
    // Up left.
    while (square_copy >= 0 && square_copy % 8 != 0)
    {
        bit_mask <<= 9;
        square_copy -= 9;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move up-right direction
    while (square_copy >= 0 && square_copy % 8 != 7)
    {
        bit_mask <<= 7;
        square_copy -= 7;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-right direction
    while (square_copy < 64 && square_copy % 8 != 7)
    {
        bit_mask >>= 9;
        square_copy += 9;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-left direction
    while (square_copy < 64 && square_copy % 8 != 0)
    {
        bit_mask >>= 7;
        square_copy += 7;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }
    
    return move_board;
}

// Rook move logic.
uint64_t make_rook_mask(uint8_t square, uint64_t occupation) 
{
    uint64_t move_board = 0b0;
    int square_copy = square;
    uint64_t bit_mask = 1ULL << (63-square);
    // Continue until another piece is found. Repeat for each direction.
    // left.
    while (square_copy >= 0 && square_copy % 8 != 0)
    {
        bit_mask <<= 1;
        square_copy--;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move up.
    while (square_copy >= 0)
    {
        bit_mask <<= 8;
        square_copy -= 8;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move right.
    while (square_copy < 64 && square_copy % 8 != 7)
    {
        bit_mask >>= 1;
        square_copy ++;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down.
    while (square_copy < 64)
    {
        bit_mask >>= 8;
        square_copy += 8;
        move_board |= bit_mask;
        if (occupation & bit_mask)
            break;  // Stop if a piece is found
    }

    return move_board;
}

static inline uint8_t get_ls1b_index(uint64_t bitboard)
{
    if (bitboard)
    {
        return __builtin_popcountll((bitboard & -bitboard) - 1);
    }
    else
        return -1;
}

uint64_t set_occupancy(int index, int bits_in_mask, uint64_t mask) 
{
    uint64_t occupancy = 0ULL;
    int bit_position = 0;
    
    // Loop through bits in mask
    for (int count = 0; count < bits_in_mask; count++) {
        // Get the least significant bit in the mask
        bit_position = __builtin_ctzll(mask);
        
        // If the corresponding bit in the index is set, set this bit in the occupancy
        if (index & (1 << count)) {
            occupancy |= (1ULL << bit_position);
        }
        
        // Remove the bit from the mask
        mask &= mask - 1;
    }
    
    return occupancy;
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