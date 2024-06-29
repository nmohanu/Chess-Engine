#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <assert.h>
#include <bitset>
#include "magic_arrays.hpp"

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

// bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
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

uint64_t make_bishop_mask(uint8_t square, uint64_t occupation);
uint64_t make_rook_mask(uint8_t square, uint64_t occupation);
uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask);

// print bitboard
void print_bitboard(uint64_t bitboard);
