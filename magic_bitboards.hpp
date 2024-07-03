#include "defenitions.hpp"

constexpr uint64_t make_bishop_mask(uint8_t square)
{
    square = 63-square;
    // result attacks bitboard
    uint64_t attacks = 0ULL;
    
    // init ranks & files
    int r = 0;
    int f = 0;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // mask relevant bishop occupancy bits
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
    
    // return attack map
    return attacks;
}

constexpr uint64_t make_rook_mask(uint8_t square)
{
    square = 63-square;
    // result attacks bitboard
    uint64_t attacks = 0ULL;
    
    // init ranks & files
    int r = 0;
    int f = 0;
    
    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;
    
    // mask relevant rook occupancy bits
    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
    
    // return attack map
    return attacks;
}

// Bishop moving logic.
constexpr uint64_t bishop_attack_on_fly(uint8_t square, uint64_t occupation) 
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
constexpr uint64_t rook_attack_on_fly(uint8_t square, uint64_t occupation) 
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


constexpr static std::array<uint64_t, 64> bishop_masks = []() 
{
    std::array<uint64_t, 64> values{};
    for(int square = 0; square < 64; square++)
    {
        // Make masks for all squares. 
        values[square] = make_bishop_mask(square);
    }
    return values;
}();

constexpr static std::array<uint64_t, 64> rook_masks = []() 
{
    std::array<uint64_t, 64> values{};
    for(int square = 0; square < 64; square++)
    {
        // Make masks for all squares. 
        values[square] = make_rook_mask(square);
    }
    return values;
}();

// Max possible permutations for a mask.
constexpr int max_permutations_rooks = 4096;
constexpr int max_permutations_bishops = 512;

// Create all mask permutations for blocker squares for rooks.
constexpr std::array<uint64_t, max_permutations_rooks> create_all_rook_perms(uint64_t movement_mask) {
    std::array<uint64_t, max_permutations_rooks> blocker_boards{};
    std::array<int, 64> move_indices{};
    int move_count = 0;

    // Loop over all squares and check if piece has range here.
    for (int square = 0; square < 64; ++square) {
        // Check if square intersects with movement mask.
        if ((movement_mask >> square) & 1) {
            move_indices[move_count++] = square;
        }
    }

    // Calculate the number of possible permutations.
    int perm_amount = 1 << move_count;

    // Create all possible blocker boards.
    for (int perm_index = 0; perm_index < perm_amount; ++perm_index) {
        uint64_t blocker_board = 0;
        for (int bit_index = 0; bit_index < move_count; ++bit_index) {
            int bit = (perm_index >> bit_index) & 1;
            blocker_board |= static_cast<uint64_t>(bit) << move_indices[bit_index];
        }
        blocker_boards[perm_index] = blocker_board;
    }

    return blocker_boards;
}

// Create all mask permutations for blocker squares for bishops.
constexpr std::array<uint64_t, max_permutations_bishops> create_all_bishop_perms(uint64_t movement_mask) {
    std::array<uint64_t, max_permutations_bishops> blocker_boards{};
    std::array<int, 64> move_indices{};
    int move_count = 0;

    // Loop over all squares and check if piece has range here.
    for (int square = 0; square < 64; ++square) {
        // Check if square intersects with movement mask.
        if ((movement_mask >> square) & 1) {
            move_indices[move_count++] = square;
        }
    }

    // Calculate the number of possible permutations.
    int perm_amount = 1 << move_count;

    // Create all possible blocker boards.
    for (int perm_index = 0; perm_index < perm_amount; ++perm_index) {
        uint64_t blocker_board = 0;
        for (int bit_index = 0; bit_index < move_count; ++bit_index) {
            int bit = (perm_index >> bit_index) & 1;
            blocker_board |= static_cast<uint64_t>(bit) << move_indices[bit_index];
        }
        blocker_boards[perm_index] = blocker_board;
    }

    return blocker_boards;
}

// Make lookup table for rooks.
constexpr static std::array<std::array<uint64_t, 4096>, 64> rook_attacks = []() {
    std::array<std::array<uint64_t, 4096>, 64> values{};
    for (int square = 0; square < 64; square++) {
        uint64_t rook_mask = rook_masks[square];
        int rook_relevant_bits = __builtin_popcountll(rook_mask);
        auto rook_blocker_boards = create_all_rook_perms(rook_mask);
        for (int board = 0; board < 4096; ++board) {
            uint64_t blocker_board = rook_blocker_boards[board];
            int index = (blocker_board * rook_magic_numbers[63 - square]) >> (64 - rook_relevant_bits);
            values[square][index] = rook_attack_on_fly(square, blocker_board);
        }
    }
    return values;
}();

// Make lookup table for bishops.
constexpr static std::array<std::array<uint64_t, 512>, 64> bishop_attacks = []() {
    std::array<std::array<uint64_t, 512>, 64> values{};
    for (int square = 0; square < 64; square++) {
        uint64_t bishop_mask = bishop_masks[square];
        int bishop_relevant_bits = __builtin_popcountll(bishop_mask);
        auto bishop_blocker_boards = create_all_bishop_perms(bishop_mask);
        for (int board = 0; board < 512; ++board) {
            uint64_t blocker_board = bishop_blocker_boards[board];
            int index = (blocker_board * bishop_magic_numbers[63 - square]) >> (64 - bishop_relevant_bits);
            values[square][index] = bishop_attack_on_fly(square, blocker_board);
        }
    }
    return values;
}();