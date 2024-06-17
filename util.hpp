#include <cstdint>
#include <iostream>
#include <vector>

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

void toggle_bit(uint64_t &num, uint64_t pos);

bool get_bit(uint64_t num, uint64_t pos);