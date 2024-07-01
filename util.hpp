#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <assert.h>
#include <bitset>
#include <chrono>
#include "magic_arrays.hpp"

typedef uint64_t (*generator_function) (uint8_t, bool, uint64_t, uint64_t);

// Define a number for each piece.
#define W_KING      0
#define W_QUEEN     1
#define W_ROOK      2
#define W_BISHOP    3
#define W_KNIGHT    4
#define W_PAWN      5
#define B_KING      6
#define B_QUEEN     7
#define B_ROOK      8
#define B_BISHOP    9
#define B_KNIGHT    10
#define B_PAWN      11
#define TOTAL       12
#define COLOR_BOARD 13
#define EMPTY       14
#define INVALID     15

#define MAX_EVAL    100.f
#define MIN_EVAL    -100.f

// Transposition table.

#define hashfEXACT  0
#define hashfALPHA  1
#define hashfBETA   2

// Table size.
#define hash_table_size 0x2000000

// No entry found.
#define no_hash_entry 999999999

const uint64_t ROOK_SQUARES =    0b1000000100000000000000000000000000000000000000000000000010000001ULL;
const uint64_t KNIGHT_SQUARES =  0b0100001000000000000000000000000000000000000000000000000001000010ULL;
const uint64_t BISHOP_SQUARES =  0b0010010000000000000000000000000000000000000000000000000000100100ULL;
const uint64_t QUEEN_SQUARES =   0b0001000000000000000000000000000000000000000000000000000000010000ULL;
const uint64_t KING_SQUARES =    0b0000100000000000000000000000000000000000000000000000000000001000ULL;
const uint64_t PAWN_SQUARES =    0b0000000011111111000000000000000000000000000000001111111100000000ULL;
const uint64_t BLACK_PIECES =    0b1111111111111111000000000000000000000000000000000000000000000000ULL;
const uint64_t TOTAL_SQUARES =   0b1111111111111111000000000000000000000000000000001111111111111111ULL;

// Whether there is a search going on or not.
static std::atomic<bool> engine_is_searching(false);
static std::atomic<bool> move_found(false);

// Constants.
const float ROOK_VALUE = 5.f;
const float QUEEN_VALUE = 9.f;
const float KNIGHT_VALUE = 3.f;
const float BISHOP_VALUE = 3.f;
const float PAWN_VALUE = 1.f;

const int PERFT_DEPTH = 8;

// Avoid collissions by only hashing and checking at nodes that are worth hashing.
const int MAX_HASH_DEPTH = 4;
const int MIN_HASH_DEPTH = 2;

const int shift_correction[4] = {0, 16, 32, 48};

const float PAWN_BONUS[64] = 
{
    0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,
    5.0f,   5.0f,   5.0f,   5.0f,   5.0f,   5.0f,   5.0f,   5.0f,
    1.0f,   1.0f,   2.0f,   3.0f,   3.0f,   2.0f,   1.0f,   1.0f,
    0.5f,   0.5f,   1.0f,   2.5f,   2.5f,   1.0f,   0.5f,   0.5f,
    0.0f,   0.0f,   0.0f,   2.0f,   2.0f,   0.0f,   0.0f,   0.0f,
    0.5f,   -0.5f,  -1.0f,  0.0f,   0.0f,   -1.0f,  -0.5f,  0.5f,
    0.5f,   1.0f,   1.0f,   -2.0f,  -2.0f,  1.0f,   1.0f,   0.5f,
    0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f,   0.0f
};

const float KNIGHT_BONUS[64] = 
{
    -5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f,
    -4.0f, -2.0f,  0.0f,  0.5f,  0.5f,  0.0f, -2.0f, -4.0f,
    -3.0f,  0.5f,  1.0f,  2.0f,  2.0f,  1.0f,  0.5f, -3.0f,
    -3.0f,  0.0f,  2.0f,  2.5f,  2.5f,  2.0f,  0.0f, -3.0f,
    -3.0f,  0.5f,  1.5f,  2.5f,  2.5f,  1.5f,  0.5f, -3.0f,
    -3.0f,  0.0f,  1.0f,  1.5f,  1.5f,  1.0f,  0.0f, -3.0f,
    -4.0f, -2.0f,  0.0f,  0.0f,  0.0f,  0.0f, -2.0f, -4.0f,
    -5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f
};

const float BISHOP_BONUS[64] = 
{
    -2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f,
    -1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.0f, -1.0f,
    -1.0f,  0.5f,  0.5f,  1.0f,  1.0f,  0.5f,  0.5f, -1.0f,
    -1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f,
    -1.0f,  0.5f,  1.0f,  1.0f,  1.0f,  1.0f,  0.5f, -1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
    -1.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.5f, -1.0f,
    -2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f
};

const float ROOK_BONUS[64] = 
{
     0.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
    -0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
    -0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
    -0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
    -0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
     0.5f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.5f,
     0.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f
};

const float QUEEN_BONUS[64] = 
{
    -2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f,
    -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
    -1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f,
    -0.5f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f,
     0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f,
    -1.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f,
    -1.0f,  0.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
    -2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f
};

const float KING_BONUS[64] = 
{
    -3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
    -3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
    -3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
    -3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
    -2.0f, -3.0f, -3.0f, -4.0f, -4.0f, -3.0f, -3.0f, -2.0f,
    -1.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -1.0f,
     2.0f,  2.0f,  0.0f,  0.0f,  0.0f,  0.0f,  2.0f,  2.0f,
     2.0f,  3.0f,  1.0f,  0.0f,  0.0f,  1.0f,  3.0f,  2.0f
};

const float KING_BONUS_ENDGAME[64] = 
{
    -5.0f, -4.0f, -3.0f, -2.0f, -2.0f, -3.0f, -4.0f, -5.0f,
    -3.0f, -2.0f, -1.0f,  0.0f,  0.0f, -1.0f, -2.0f, -3.0f,
    -3.0f, -1.0f,  2.0f,  3.0f,  3.0f,  2.0f, -1.0f, -3.0f,
    -3.0f, -1.0f,  3.0f,  4.0f,  4.0f,  3.0f, -1.0f, -3.0f,
    -3.0f, -1.0f,  3.0f,  4.0f,  4.0f,  3.0f, -1.0f, -3.0f,
    -3.0f, -1.0f,  2.0f,  3.0f,  3.0f,  2.0f, -1.0f, -3.0f,
    -3.0f, -3.0f,  0.0f,  0.0f,  0.0f,  0.0f, -3.0f, -3.0f,
    -5.0f, -3.0f, -3.0f, -3.0f, -3.0f, -3.0f, -3.0f, -5.0f
};

// Check if a certain bit is on or off. (64bit)
inline bool get_bit_64(uint64_t num, uint8_t pos) 
{
    return (num & (1ULL << (63 - pos))) != 0;
}

// Toggle bit on.
inline void toggle_bit_on(uint64_t &num, uint64_t pos)
{
    assert(pos < 64);
    uint64_t mask = 1ULL << (63 - pos);
    num |= mask;
}

// Toggle bit off.
inline void toggle_bit_off(uint64_t &num, uint64_t pos)
{
    if(pos >= 64)
        std::cout << "ehhmhmm";
    assert(pos < 64);
    uint64_t mask = 1ULL << (63 - pos);
    mask = ~mask;
    num &= mask;
}

// Check if a certain bit is on or off.
inline bool get_bit(uint8_t num, uint8_t pos)
{
    uint8_t mask = 1ULL << (7 - pos);
    uint8_t result = (num & mask);
    return result;
}


// Useful for debugging.
inline void print_binary(uint64_t num)
{
    std::cout << "Binary representation: ";
    std::cout << std::bitset<sizeof(num) * 8>(num) << std::endl;
}

// Get the value of a piece.
inline float get_piece_value(uint8_t piece)
{

    switch (piece) {
        case W_PAWN:
        case B_PAWN:
            return PAWN_VALUE;
        case W_KNIGHT:
        case B_KNIGHT:
            return KNIGHT_VALUE;
        case W_BISHOP:
        case B_BISHOP:
            return BISHOP_VALUE;
        case W_ROOK:
        case B_ROOK:
            return ROOK_VALUE;
        case W_QUEEN:
        case B_QUEEN:
            return QUEEN_VALUE;
        default:
            return 0.0;
    }
}

inline bool boards_intersect(uint64_t one, uint64_t two)
{
    return (one & two) != 0;
}

inline uint8_t find_bit_position(uint64_t num)
{
    return 63-__builtin_ctzll(num);
}

inline std::string make_chess_notation(int index) 
{
    // Convert column index (0-7) to letter (a-h)
    char column = 'a' + (index % 8);
    // Convert row index (0-7) to number (1-8)
    char row = '8' - (index / 8);
    return std::string(1, column) + std::string(1, row);
}

inline uint64_t get_pawn_attack(bool is_black, uint8_t pos, uint64_t color_board, uint64_t occupancies)
{
    return  is_black ? (1ULL << (63-pos-9) | 1ULL << (63-pos-7)) & (0xFFULL << (64-(pos - pos%8) - 16) & ~(color_board) & occupancies) 
        : (1ULL << (63-pos+9) | 1ULL << (63-pos + 7)) & (0xFFULL << (64-(pos - pos%8)) & color_board);;
}

inline int chess_notation_to_index(const std::string& notation)
{
    if (notation.length() != 2)
    {
        throw std::invalid_argument("Invalid chess notation");
    }

    char column = notation[0];
    char row = notation[1];

    // Convert column letter (a-h) to index (0-7)
    int column_index = column - 'a';
    if (column_index < 0 || column_index > 7)
    {
        throw std::invalid_argument("Invalid column in chess notation");
    }

    // Convert row number (1-8) to index (0-7)
    int row_index = '8' - row;
    if (row_index < 0 || row_index > 7)
    {
        throw std::invalid_argument("Invalid row in chess notation");
    }

    // Calculate the 0-based index
    return row_index * 8 + column_index;
}

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

// print bitboard
void print_bitboard(uint64_t bitboard);

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

constexpr int max_permutations = 4096;

constexpr std::array<uint64_t, max_permutations> create_all_blocker_boards(uint64_t movement_mask) {
    std::array<uint64_t, max_permutations> blocker_boards{};
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
        auto rook_blocker_boards = create_all_blocker_boards(rook_mask);
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
        auto bishop_blocker_boards = create_all_blocker_boards(bishop_mask);
        for (int board = 0; board < 512; ++board) {
            uint64_t blocker_board = bishop_blocker_boards[board];
            int index = (blocker_board * bishop_magic_numbers[63 - square]) >> (64 - bishop_relevant_bits);
            values[square][index] = bishop_attack_on_fly(square, blocker_board);
        }
    }
    return values;
}();

// King moving.
static inline uint64_t get_king_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{
    // Get squares from memory.
    return KING_MOVE_SQUARES[square];;
}

// ==============================================================================================

// Bishop moving logic.
static inline uint64_t get_bishop_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{
    uint64_t mask = bishop_masks[square];
    uint64_t occupancy = occupancies & mask;
    occupancy *= bishop_magic_numbers[63 - square];
    occupancy >>= (64 - __builtin_popcountll(mask));

    return bishop_attacks[square][occupancy];
}

// ==============================================================================================

// Knight move logic.
static inline uint64_t get_knight_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{
    // Get squares from memory.
    uint64_t move_board = KNIGHT_MOVE_SQUARES[square];
    return move_board;
}

// ==============================================================================================

// Rook move logic.
static inline uint64_t get_rook_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{
    uint64_t mask = rook_masks[square];
    uint64_t occupancy = occupancies & mask;
    occupancy *= rook_magic_numbers[63 - square];
    occupancy >>= (64 - __builtin_popcountll(mask));
    return rook_attacks[square][occupancy];
}

// ==============================================================================================

// Queen move logic.
static inline uint64_t get_queen_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{   
    return get_bishop_move(square, is_black, occupancies, black_pieces) | get_rook_move(square, is_black, occupancies, black_pieces);   
}

// Pawn moving.
static inline uint64_t get_pawn_move(uint8_t square, bool is_black, uint64_t occupancies, uint64_t black_pieces) 
{
    uint64_t move_board = 0b0;
    
    // Check if the pawn is white or black.
    if (!is_black) 
    {   // White pawn
        move_board |= (1ULL << ((63-square)+8) & ~occupancies);
        // Check if pawn can move 2 squares. Only if in initial position and target square is empty.
        move_board |= ((move_board << 8) & (0xFFULL << 24)) & ~occupancies;
        // Attacking squares.
        move_board |= (1ULL << (63-square+9) | 1ULL << (63-square + 7)) & (0xFFULL << (64-(square - square%8)) & black_pieces);
    } 
    else 
    {   // Black pawn
        move_board |= 1ULL << ((63-square)-8) & ~occupancies;
        // Check if pawn can move 2 squares. Only if in initial position and target square is empty.
        move_board |= ((move_board >> 8) & (0xFFULL << 32)) & ~occupancies;
        // Attacking squares.
        move_board |= (1ULL << (63-square-9) | 1ULL << (63-square-7)) & (0xFFULL << (64-(square - square%8) - 16) & ~(black_pieces) & occupancies);
    }
    
    return move_board;
}