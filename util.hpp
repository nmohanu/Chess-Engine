#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>

// 0000 empty.

// white:

// 0001 king.
// 0010 queen.
// 0011 rook.
// 0100 bishop.
// 0101 knight.
// 0110 pawn. 

// black:

// 1001 king.
// 1010 queen.
// 1011 rook.
// 1100 bishop.
// 1101 knight.
// 1110 pawn. 

#define EMPTY    0b0000
#define W_KING   0b0001
#define W_QUEEN  0b0010
#define W_ROOK   0b0011
#define W_BISHOP 0b0100
#define W_KNIGHT 0b0101
#define W_PAWN   0b0110
#define B_KING   0b1001
#define B_QUEEN  0b1010
#define B_ROOK   0b1011
#define B_BISHOP 0b1100
#define B_KNIGHT 0b1101
#define B_PAWN   0b1110

// Black side.
#define FIRST_16_SQUARES  0b1011'1101'1100'1010'1001'1100'1101'1011'1110'1110'1110'1110'1110'1110'1110'1110
#define SECOND_16_SQUARES  0b0

// White side.
#define THIRD_16_SQUARES 0b0
#define FOURTH_16_SQUARES 0b0110'0110'0110'0110'0110'0110'0110'0110'0011'0101'0100'0010'0001'0100'0101'0011 

#define WHITE_PIECES (uint8_t[])[W_KING, W_QUEEN, W_BISHOP, W_KNIGHT, W_PAWN, W_ROOK]
#define BLACK_PIECES (uint8_t[])[B_KING, B_QUEEN, B_BISHOP, B_KNIGHT, B_PAWN, B_ROOK]

#define MAX_EVAL 999999999.f
#define MIN_EVAL -999999999.f

// Transposition table.

#define hashfEXACT 0
#define hashfALPHA 1
#define hashfBETA 2

// Table size.
#define hash_table_size 0x4000000

// No entry found.
#define no_hash_entry 999999999

// Whether there is a search going on or not.
static std::atomic<bool> engine_is_searching(false);
static std::atomic<bool> move_found(false);

// Constants.
const float ROOK_VALUE = 5.f;
const float QUEEN_VALUE = 9.f;
const float KNIGHT_VALUE = 3.f;
const float BISHOP_VALUE = 3.f;
const float PAWN_VALUE = 1.f;

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

// Toggle a bit on (but not off).
void toggle_bit_on(uint64_t &num, uint64_t pos);

// Get bit of an 8 bit number.
bool get_bit(uint8_t num, uint8_t pos);

// Get bit of a 64 bit number.
bool get_bit_64(uint64_t num, uint64_t pos);

// Convert x and y to n'th square.
int make_pos(int x, int y);

// Get the color sign of a piece.
bool get_color(uint8_t piece);

// Print a binary number (for debugging).
void print_binary(uint64_t num);

// Get value of a piece.
float get_piece_value(uint8_t piece);