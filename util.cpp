#include "util.hpp"
#include <bitset>

// Toggle bit on.
void toggle_bit_on(uint64_t &num, uint64_t pos)
{
    assert(pos < 64);
    uint64_t mask = 1ULL << (63 - pos);
    num |= mask;
}

// Toggle bit off.
void toggle_bit_off(uint64_t &num, uint64_t pos)
{
    if(pos >= 64)
        std::cout << "ehhmhmm";
    assert(pos < 64);
    uint64_t mask = 1ULL << (63 - pos);
    mask = ~mask;
    num &= mask;
}

// Check if a certain bit is on or off.
bool get_bit(uint8_t num, uint8_t pos)
{
    uint8_t mask = 1ULL << (7 - pos);
    uint8_t result = (num & mask);
    return result;
}

// Check if a certain bit is on or off. (64bit)
bool get_bit_64(uint64_t num, uint8_t pos)
{
    uint64_t mask = 1ULL << (63 - pos);
    return (num & mask) != 0;
}

// Useful for debugging.
void print_binary(uint64_t num)
{
    std::cout << "Binary representation: ";
    std::cout << std::bitset<sizeof(num) * 8>(num) << std::endl;
}

// Get the value of a piece.
float get_piece_value(uint8_t piece)
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

// Used for safety.
bool square_in_bounds(uint8_t square)
{   
    return square >= 0 && square < 64;   
}

bool boards_intersect(uint64_t one, uint64_t two)
{
    return (one & two) != 0;
}

uint8_t find_bit_position(uint64_t num)
{
    uint64_t mask = 1ULL << 63;
    for(int i = 0; i < 64; i++)
    {   
        if((mask & num) != 0)
        {
            return i;
        }
        mask >>= 1;
    }
    return 64;
}