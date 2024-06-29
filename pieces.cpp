#include "board.hpp"

// ==============================================================================================

// Moving logic: 

// ==============================================================================================

// Pawn moving.
uint64_t Position::get_pawn_move(uint8_t square, bool is_black) 
{
    uint64_t move_board = 0b0;
    
    // Check if the pawn is white or black.
    if (!is_black) 
    {   // White pawn
        move_board |= (1ULL << ((63-square)+8) & ~bit_boards[TOTAL]);
        // Check if pawn can move 2 squares. Only if in initial position and target square is empty.
        move_board |= ((move_board << 8) & (0xFFULL << 24)) & ~bit_boards[TOTAL];
        // Attacking squares.
        move_board |= (1ULL << (63-square+9) | 1ULL << (63-square + 7)) & (0xFFULL << (64-(square - square%8)) & bit_boards[COLOR_BOARD]);
    } 
    else 
    {   // Black pawn
        move_board |= 1ULL << ((63-square)-8) & ~bit_boards[TOTAL];
        // Check if pawn can move 2 squares. Only if in initial position and target square is empty.
        move_board |= ((move_board >> 8) & (0xFFULL << 32)) & ~bit_boards[TOTAL];
        // Attacking squares.
        move_board |= (1ULL << (63-square-9) | 1ULL << (63-square-7)) & (0xFFULL << (64-(square - square%8) - 16) & ~(bit_boards[COLOR_BOARD]) & bit_boards[TOTAL]);
        
    }
    
    return move_board;
}


// ==============================================================================================

// King moving.
uint64_t Position::get_king_move(uint8_t square, bool is_black) 
{
    // Get squares from memory.
    uint64_t move_board = KING_MOVE_SQUARES[square];
    if(is_black)
        move_board &= ~bit_boards[COLOR_BOARD];
    else
        move_board &= ~(~bit_boards[COLOR_BOARD] & bit_boards[TOTAL]);

    // Remove occupied squares.
    return move_board;
}

// ==============================================================================================

// Bishop moving logic.
uint64_t Position::get_bishop_move(uint8_t square, bool is_black) 
{

    uint64_t mask = bishop_masks[square];
    uint64_t occupancy = bit_boards[TOTAL] & mask;
    occupancy *= bishop_magic_numbers[63 - square];
    occupancy >>= (64 - __builtin_popcountll(mask));
    uint64_t move_board = bishop_attacks[square][occupancy];

    // Can't move to squares occupied by own color.
    if(is_black)
        move_board &= ~bit_boards[COLOR_BOARD];
    else
        move_board &= ~(~bit_boards[COLOR_BOARD] & bit_boards[TOTAL]);


    return move_board;
}

// ==============================================================================================

// Knight move logic.
uint64_t Position::get_knight_move(uint8_t square, bool is_black) 
{
    // Get squares from memory.
    uint64_t move_board = KNIGHT_MOVE_SQUARES[square];
    if(is_black)
        move_board &= ~bit_boards[COLOR_BOARD];
    else
        move_board &= ~(~bit_boards[COLOR_BOARD] & bit_boards[TOTAL]);

    return move_board;
}

// ==============================================================================================

// Rook move logic.
uint64_t Position::get_rook_move(uint8_t square, bool is_black) 
{
    uint64_t mask = rook_masks[square];
    uint64_t occupancy = bit_boards[TOTAL] & mask;
    occupancy *= rook_magic_numbers[63 - square];
    occupancy >>= (64 - __builtin_popcountll(mask));
    uint64_t move_board = rook_attacks[square][occupancy];

    // Can't move to squares occupied by own color.
    if(is_black)
        move_board &= ~bit_boards[COLOR_BOARD];
    else
        move_board &= ~(~bit_boards[COLOR_BOARD] & bit_boards[TOTAL]);

    return move_board;
}

// ==============================================================================================

// Queen move logic.
uint64_t Position::get_queen_move(uint8_t square, bool is_black) 
{   
    return get_bishop_move(square, is_black) | get_rook_move(square, is_black);   
}