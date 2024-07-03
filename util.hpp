#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <assert.h>
#include <bitset>
#include <chrono>
#include "magic_bitboards.hpp"

// ==============================================================================================

// Check if a certain bit is on or off. (64bit)
inline bool get_bit_64(uint64_t num, uint8_t pos) 
{
    return (num & (1ULL << (63 - pos))) != 0;
}

// ==============================================================================================

// Toggle bit on.
inline void toggle_bit_on(uint64_t &num, uint64_t pos)
{
    assert(pos < 64);
    uint64_t mask = 1ULL << (63 - pos);
    num |= mask;
}

// ==============================================================================================

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

// ==============================================================================================

// Check if a certain bit is on or off.
inline bool get_bit(uint8_t num, uint8_t pos)
{
    uint8_t mask = 1ULL << (7 - pos);
    uint8_t result = (num & mask);
    return result;
}

// ==============================================================================================

// Useful for debugging.
inline void print_binary(uint64_t num)
{
    std::cout << "Binary representation: ";
    std::cout << std::bitset<sizeof(num) * 8>(num) << std::endl;
}

// ==============================================================================================

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

// ==============================================================================================

inline bool boards_intersect(uint64_t one, uint64_t two)
{
    return (one & two) != 0;
}

// ==============================================================================================

inline uint8_t find_bit_position(uint64_t num)
{
    return 63-__builtin_ctzll(num);
}

// ==============================================================================================

inline std::string make_chess_notation(int index) 
{
    // Convert column index (0-7) to letter (a-h)
    char column = 'a' + (index % 8);
    // Convert row index (0-7) to number (1-8)
    char row = '8' - (index / 8);
    return std::string(1, column) + std::string(1, row);
}

// ==============================================================================================

inline uint64_t get_pawn_attack(bool is_black, uint8_t pos, uint64_t color_board, uint64_t occupancies)
{
    return  is_black ? (1ULL << (63-pos-9) | 1ULL << (63-pos-7)) & (0xFFULL << (64-(pos - pos%8) - 16) & ~(color_board) & occupancies) 
        : (1ULL << (63-pos+9) | 1ULL << (63-pos + 7)) & (0xFFULL << (64-(pos - pos%8)) & color_board);;
}

// ==============================================================================================

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

    int square = row_index * 8 + column_index;

    std::cout << "\nConverted to: " << square << '\n';

    // Calculate the 0-based index
    return square;
}

// ==============================================================================================

// print bitboard
void print_bitboard(uint64_t bitboard);

// ==============================================================================================

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