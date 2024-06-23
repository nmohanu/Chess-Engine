#include "board.hpp"

// ==============================================================================================

// Moving logic: 

// ==============================================================================================

// Pawn moving.
uint64_t Position::get_pawn_move(uint8_t square, bool is_black) {
    uint64_t move_board = 0b0;

    // Check if the pawn is white or black.
    if (!is_black && square_in_bounds(square-8) && get_piece(square-8) == EMPTY) 
    {
        // Check if pawn can move 2 squares. Only if in itial position and target square is empty.
        if (square > 47 && get_piece(square - 16) == EMPTY) 
        {
            toggle_bit_on(move_board, square - 16);
        }
        toggle_bit_on(move_board, square-8);
    } 
    // Now for black.
    else if (square_in_bounds(square+8) && get_piece(square+8) == EMPTY) 
    {
        // Check if pawn can move 2 squares. Only if in itial position and target square is empty.
        if (square < 16 && get_piece(square + 16) == EMPTY) 
        {
            toggle_bit_on(move_board, square+16);
        }
        toggle_bit_on(move_board, square+8);
    } 

    return move_board;
}

// ==============================================================================================

// King moving.
uint64_t Position::get_king_move(uint8_t square, bool is_black) 
{
    // Initialize.
    uint64_t move_board = 0b0;
    int directions[8][2] = 
    {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    // Toggle all empty bits around the king
    for (uint8_t i = 0; i < 8; i++) 
    {
        uint8_t new_square = square + directions[i][0] + 8*directions[i][1];

        if(!square_in_bounds(new_square))
            continue;
    
        // if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
        toggle_bit_on(move_board, new_square);
    }

    // King cannot move to square containing pieces of their own color.
    move_board &= (is_black) ? ~bit_boards[BLACK_PIECES] : bit_boards[BLACK_PIECES];

    // Remove occupied squares.
    return move_board;
}

// ==============================================================================================

// Bishop moving logic.
uint64_t Position::get_bishop_move(uint8_t square, bool is_black) 
{
    uint64_t move_board = 0b0;

    // Define the directions as square increments.
    int directions[4][2] = 
    {
        {-1, -1},   {-1, 1},
        { 1, -1},   { 1, 1}
    };

    // Loop over directions.
    for (uint8_t dir = 0; dir < 4; dir++) 
    {   
        uint8_t offset = 1;
        // Continue until another piece is found.
        while (true)
        {
            // Calculate square to check.
            uint8_t new_square = square + directions[dir][0]*offset + 8*directions[dir][1]*offset;

            // Square may be off board.
            if(!square_in_bounds(square-8))
                continue;

            // Toggle bit on.
            toggle_bit_on(move_board, new_square);

            // If a piece is found, stop looking in this direction.
            if(get_bit(bit_boards[TOTAL], new_square))
                break;

            // Look one square further.
            offset++;
        }
    }
    // Can't move to squares occupied by own color.
    move_board &= (is_black) ? ~bit_boards[BLACK_PIECES] : bit_boards[BLACK_PIECES];
    return move_board;
}

// ==============================================================================================

// Knight move logic.
uint64_t Position::get_knight_move(uint8_t square, bool is_black) 
{
    uint64_t move_board = 0b0;

    // Define directions.
    int directions[8][2] = 
    {
        { 1,  2}, { 1, -2}, {-1,  2}, {-1, -2},
        { 2,  1}, { 2, -1}, {-2,  1}, {-2, -1}
    };

    // Loop through candidate squares.
    for (int i = 0; i < 8; ++i) 
    {
        uint8_t new_square = square + directions[i][0] + 8*directions[i][1];

        // Square may be off bounds.
        if(!square_in_bounds(new_square))
            continue;

        if (this->get_piece(new_square) == EMPTY)
            toggle_bit_on(move_board, new_square);
    }

    // Can't move to squares occupied by own color.
    move_board &= (is_black) ? ~bit_boards[BLACK_PIECES] : bit_boards[BLACK_PIECES];
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
            // Calculate target square.
            uint8_t new_square = square + directions[dir][0]*offset + 8*directions[dir][1]*offset;

            if(!square_in_bounds(new_square))
                continue;

            toggle_bit_on(move_board, new_square);

            if (get_piece(new_square) != EMPTY)
                break;

            offset++;
        }
    }
    // Can't move to squares occupied by own color.
    move_board &= (is_black) ? ~bit_boards[BLACK_PIECES] : bit_boards[BLACK_PIECES];
    return move_board;
}

// ==============================================================================================

// Queen move logic.
uint64_t Position::get_queen_move(uint8_t square, bool is_black) 
{   
    return get_bishop_move(square, is_black) | get_rook_move(square, is_black);   
}