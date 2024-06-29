#include "util.hpp"

// Bishop moving logic.
uint64_t bishop_attack_on_fly(uint8_t square, uint64_t occupation) 
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

uint64_t make_bishop_mask(uint8_t square)
{
    uint64_t move_board = 0b0;

    int square_copy = square;
    uint64_t bit_mask = 1ULL << (63-square);
    // Continue until another piece is found. Repeat for each direction.
    // Up left.
    while (square_copy >= 16 && square_copy % 8 != 1)
    {
        bit_mask <<= 9;
        square_copy -= 9;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move up-right direction
    while (square_copy >= 16 && square_copy % 8 != 6)
    {
        bit_mask <<= 7;
        square_copy -= 7;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-right direction
    while (square_copy < 48 && square_copy % 8 != 6)
    {
        bit_mask >>= 9;
        square_copy += 9;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-left direction
    while (square_copy < 48 && square_copy % 8 != 1)
    {
        bit_mask >>= 7;
        square_copy += 7;
        move_board |= bit_mask;
    }

    return move_board;
}

// Rook move logic.
uint64_t rook_attack_on_fly(uint8_t square, uint64_t occupation) 
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

uint64_t make_rook_mask(uint8_t square)
{
    uint64_t move_board = 0b0;
    int square_copy = square;
    uint64_t bit_mask = 1ULL << (63-square);
    // Continue until another piece is found. Repeat for each direction.
    // left.
    while (square_copy >= 0 && square_copy % 8 != 1)
    {
        bit_mask <<= 1;
        square_copy--;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move up.
    while (square_copy >= 16)
    {
        bit_mask <<= 8;
        square_copy -= 8;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move right.
    while (square_copy < 63 && square_copy % 8 != 6)
    {
        bit_mask >>= 1;
        square_copy ++;
        move_board |= bit_mask;
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down.
    while (square_copy < 48)
    {
        bit_mask >>= 8;
        square_copy += 8;
        move_board |= bit_mask;
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

std::vector<uint64_t> create_all_blocker_boards(uint64_t movement_mask)
{
    std::vector<uint64_t> blocker_boards;

    std::vector<int> move_indices;

    // Loop over all squares and check if piece has range here.
    for(int square = 0; square < 64; square++)
    {
        // Check if square intersects with movement mask.
        if(((movement_mask >> square) & 1))
        {
            move_indices.push_back(square);
        }
    }

    // Calculate the amount of possible permutations.
    uint16_t perm_amount = 1ULL << move_indices.size();

    // Create all possible blockerboards.
    for(int perm_index = 0; perm_index < perm_amount; perm_index++)
    {
        blocker_boards.push_back(0);
        for(int bit_index = 0; bit_index < move_indices.size(); bit_index++)
        {
            int bit = (perm_index >> bit_index) & 1;
            blocker_boards[perm_index] |= (uint64_t) bit << move_indices[bit_index];
            
        }
        // Debug if needed.
        // print_bitboard(blocker_boards[perm_index]);
    }

    return blocker_boards;
}