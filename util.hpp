#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <assert.h>

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
#define hash_table_size 0x400000

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

// Toggle a bit on (but not off).
void toggle_bit_on(uint64_t &num, uint64_t pos);
// Toggle a bit off (but not on).
void toggle_bit_off(uint64_t &num, uint64_t pos);

// Get bit of an 8 bit number.
bool get_bit(uint8_t num, uint8_t pos);

// Get bit of a 64 bit number.
bool get_bit_64(uint64_t num, uint8_t pos);

// Print a binary number (for debugging).
void print_binary(uint64_t num);

// Get value of a piece.
float get_piece_value(uint8_t piece);

// Check if square is on the board;
bool square_in_bounds(uint8_t square);

// Check if two bitboards intersect.
bool boards_intersect(uint64_t one, uint64_t two);

uint8_t find_bit_position(uint64_t num);