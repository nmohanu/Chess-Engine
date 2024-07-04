#include "util.hpp"
#include <iomanip>
#include <algorithm> 
#include <cassert>

struct Position;

struct Move
{   
    // ==============================================================================================

    // Constructors.
    Move();
    Move(Move* other);
    Move(uint8_t start, uint8_t end) : start_location(start), end_location(end) {}

    // ==============================================================================================

    // Move functions.
    bool is_check(Position* position) const;
    bool is_capture(Position* position) const;
    float capture_value(Position* position) const;
    bool move_bounds_valid();

    // ==============================================================================================

    // Util.
    std::string to_string();

    // ==============================================================================================

    // Move data.
    uint8_t start_location;
    uint8_t end_location;
    uint8_t moving_piece = INVALID;
    uint8_t captured_piece = INVALID;
    uint8_t previous_en_passant;
    uint8_t previous_castling_rights;
    // Move might be a castling move or engine needs to check for en passant next move.
    // 1 = white kingside, 2 = white queenside, 3 = black kingside, 4 = black queenside, 5 = engine needs to check for an passant afterwards.
    uint8_t special_cases = 0b0;
    // TODO: remove this, we save status as uint8.
    bool move_takes_an_passant = false;
    // If a move is a promition.
    // 1: Queen. 2: rook. 3: bishop. 4: knight.
    int promotion = 0;

    // ==============================================================================================

    // By engine used variables.
    // For move sorting. Lower: more promising.
    int priority_group = 5;

    // Evaluation of position after this move.
    float evaluation;
};

// ==============================================================================================

// Moves struct, keep array with possible moves.
typedef struct 
{
    Move moves[1024];
    int move_count;
} moves;

// ==============================================================================================