#include "move.hpp"

// ==============================================================================================

// Move constructors.
Move::Move() {}

// ==============================================================================================

// Copy constructor.
Move::Move(Move* other)
{
    // Copy contents.
    this->start_location = other->start_location;
    this->end_location = other->end_location;
    this->special_cases = other->special_cases;
    this->move_takes_an_passant = other->move_takes_an_passant;
    this->evaluation = other->evaluation;
    this->moving_piece = other->moving_piece;
    this->captured_piece = other->captured_piece;
    this->previous_en_passant = other->previous_en_passant;
    this->promotion = other->promotion;
}

// ==============================================================================================

// Function to check if the bounds of a move are valid.
bool Move::move_bounds_valid()
{
    return  (start_location >= 0 && start_location < 64 && end_location >= 0 && end_location < 64);
}

// ==============================================================================================