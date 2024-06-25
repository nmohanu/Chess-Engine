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
        // Attacking squares.)
        move_board |= (1ULL << (63-square-9) | 1ULL << (63-square-7)) & (0xFFULL << (64-(square - square%8) - 16) & ~(bit_boards[COLOR_BOARD]) & bit_boards[TOTAL]);
    }
    
    return move_board;
}


// ==============================================================================================

// King moving.
uint64_t Position::get_king_move(uint8_t square, bool is_black) 
{
    // Initialize.
    uint64_t move_board = KING_MOVE_SQUARES[square];

    // uint64_t move_board = 0b0;
    // int directions[8][2] = 
    // {
    //     {-1, -1}, {-1, 0}, {-1, 1},
    //     { 0, -1},          { 0, 1},
    //     { 1, -1}, { 1, 0}, { 1, 1}
    // };

    // // Toggle all empty bits around the king
    // for (uint8_t i = 0; i < 8; i++) 
    // {
    //     uint8_t new_square = square + directions[i][0] + 8*directions[i][1];

    //     if(!square_in_bounds(new_square))
    //         continue;
    
    //     // if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
    //     toggle_bit_on(move_board, new_square);
    // }

    // Can't move to squares occupied by own color.

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
            int x = square % 8 + directions[dir][0]*offset;
            int y = square / 8 + directions[dir][1]*offset;
            // Square may be off board.
            if(x < 0 || x >= 8 || y < 0 || y >= 8)
                break;

            // Calculate square to check.
            uint8_t new_square = x + 8*y;

            // Toggle bit on.
            assert(square_in_bounds(new_square));
            toggle_bit_on(move_board, new_square);

            // If a piece is found, stop looking in this direction.
            if(get_bit_64(bit_boards[TOTAL], new_square))
                break;

            // Look one square further.
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

// Knight move logic.
uint64_t Position::get_knight_move(uint8_t square, bool is_black) 
{
    uint64_t move_board = KNIGHT_MOVE_SQUARES[square];

    // uint64_t move_board = 0b0;
    // // Define directions.
    // int directions[8][2] = 
    // {
    //     { 1,  2}, { 1, -2}, {-1,  2}, {-1, -2},
    //     { 2,  1}, { 2, -1}, {-2,  1}, {-2, -1}
    // };

    // // Loop through candidate squares.
    // for (int i = 0; i < 8; ++i) 
    // {
    //     int x = square % 8 + directions[i][0];
    //     int y = square / 8 + directions[i][1];
    //     // Square may be off board.
    //     if(x < 0 || x >= 8 || y < 0 || y >= 8)
    //         continue;;

    //     // Calculate square to check.
    //     uint8_t new_square = x + 8*y;

    //     assert(square_in_bounds(new_square));
    //     toggle_bit_on(move_board, new_square);
    // }

    // Can't move to squares occupied by own color.
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