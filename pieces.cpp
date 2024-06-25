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
    uint64_t move_board = 0b0;

    uint8_t offset = 1;
    int square_copy = square;
    uint64_t bit_mask = 1ULL << (63-square);
    // Continue until another piece is found. Repeat for each direction.
    while (square_copy >= 0 && square_copy % 8 != 0)
    {
        bit_mask <<= 9;
        square_copy -= 9;
        move_board |= bit_mask;
        if (bit_boards[TOTAL] & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move up-left direction
    while (square_copy >= 0 && square_copy % 8 != 7)
    {
        bit_mask <<= 7;
        square_copy -= 7;
        move_board |= bit_mask;
        if (bit_boards[TOTAL] & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-left direction
    while (square_copy < 64 && square_copy % 8 != 7)
    {
        bit_mask >>= 9;
        square_copy += 9;
        move_board |= bit_mask;
        if (bit_boards[TOTAL] & bit_mask)
            break;  // Stop if a piece is found
    }

    square_copy = square;
    bit_mask = 1ULL << (63 - square);

    // Move down-right direction
    while (square_copy < 64 && square_copy % 8 != 0)
    {
        bit_mask >>= 7;
        square_copy += 7;
        move_board |= bit_mask;
        if (bit_boards[TOTAL] & bit_mask)
            break;  // Stop if a piece is found
    }
    
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
    uint64_t move_board = 0b0;
    int directions[4][2] =
    {
        {-1, 0}, {1, 0},
        {0, -1}, {0, 1}
    };

    // Loop through directions. 
    for (int dir = 0; dir < 4; dir++) 
    {
        uint8_t offset = 1;

        while (true) 
        {
            int x = square % 8 + directions[dir][0]*offset;
            int y = square / 8 + directions[dir][1]*offset;
            // Square may be off board.
            if(x < 0 || x >= 8 || y < 0 || y >= 8)
                break;

            // Calculate square to check.
            uint8_t new_square = x + 8*y;

            assert(square_in_bounds(new_square));
            toggle_bit_on(move_board, new_square);

            // If a piece is found, stop looking in this direction.
            if(get_bit_64(bit_boards[TOTAL], new_square))
                break;

            offset++;
        }
    }
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