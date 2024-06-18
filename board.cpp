#include "board.hpp"
#include <algorithm> 

void Position::initialize()
{
    this->first_16 = FIRST_16_SQUARES;
    this->second_16 = SECOND_16_SQUARES;
    this->third_16 = THIRD_16_SQUARES;
    this->fourth_16 = FOURTH_16_SQUARES;
}

Board::Board()
{
    position = new Position();
    position->initialize();
}

Board::~Board()
{
    delete position;
}

Position::~Position()
{
    delete last_move;
}

// Create attack board for specific piece. This is where we define the attacking logic for each piece.
uint64_t Position::make_reach_board(uint8_t x, uint8_t y)
{
    uint64_t attack_board = 0b0;

    int pos = make_pos(x, y);

    uint8_t piece_type = this->get_piece(pos);

    // This is where we determine where a piece can go.
    if(piece_type == B_PAWN) 
    {
        if((x+1) <= 7 && (y+1) <= 7)
            toggle_bit_on(attack_board, make_pos(std::min(7, x+1), std::min(7, y+1)));
        if((x-1) >= 0 && (y+1) <= 7)
            toggle_bit_on(attack_board, make_pos(std::min(7, x-1), std::min(7, y+1)));
    }
    else if(piece_type == W_PAWN)
    {
        if((x+1) <= 7 && (y-1) > 0)
            toggle_bit_on(attack_board, make_pos(std::min(7, x+1), std::min(7, y-1)));
        if((x-1) >= 0 && (y-1) > 0)
            toggle_bit_on(attack_board, make_pos(std::min(7, x-1), std::min(7, y-1)));
    }
    else if(piece_type == B_KING || piece_type == W_KING)
    {
        int offsets[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };
        for (int i = 0; i < 8; ++i) {
            int new_x = x + offsets[i][0];
            int new_y = y + offsets[i][1];
            
            // Check if the new position is within bounds
            if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
                int pos = make_pos(new_x, new_y);
                toggle_bit_on(attack_board, pos);
            }
        }
    }
    if (piece_type == B_BISHOP || piece_type == W_BISHOP || piece_type == W_QUEEN || piece_type == B_QUEEN) {
        int xDeltas[] = {-1, 1, -1, 1};
        int yDeltas[] = {-1, -1, 1, 1};

        // Loop through each diagonal direction.
        for (int dir = 0; dir < 4; ++dir) {
            int xIt = x + xDeltas[dir];
            int yIt = y + yDeltas[dir];

            // Check each square along the diagonal until edge of board or another piece is encountered.
            while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
                uint8_t pos_to_check = make_pos(xIt, yIt);
                toggle_bit_on(attack_board, pos_to_check);

                if (get_piece(pos_to_check) != 0)
                    break;

                // Move to the next square in the current diagonal direction.
                xIt += xDeltas[dir];
                yIt += yDeltas[dir];
            }
        }
    }

    else if (piece_type == B_KNIGHT || piece_type == W_KNIGHT)
    {
        if (x + 1 >= 0 && x + 1 <= 7 && y + 2 >= 0 && y + 2 <= 7)
            toggle_bit_on(attack_board, make_pos(x + 1, y + 2));

        if (x - 1 >= 0 && x - 1 <= 7 && y + 2 >= 0 && y + 2 <= 7)
            toggle_bit_on(attack_board, make_pos(x - 1, y + 2));

        if (x + 1 >= 0 && x + 1 <= 7 && y - 2 >= 0 && y - 2 <= 7)
            toggle_bit_on(attack_board, make_pos(x + 1, y - 2));

        if (x - 1 >= 0 && x - 1 <= 7 && y - 2 >= 0 && y - 2 <= 7)
            toggle_bit_on(attack_board, make_pos(x - 1, y - 2));

        if (x + 2 >= 0 && x + 2 <= 7 && y + 1 >= 0 && y + 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x + 2, y + 1));

        if (x + 2 >= 0 && x + 2 <= 7 && y - 1 >= 0 && y - 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x + 2, y - 1));

        if (x - 2 >= 0 && x - 2 <= 7 && y + 1 >= 0 && y + 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x - 2, y + 1));

        if (x - 2 >= 0 && x - 2 <= 7 && y - 1 >= 0 && y - 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x - 2, y - 1));

    }
    if (piece_type == W_QUEEN || piece_type == B_QUEEN || piece_type == W_ROOK || piece_type == B_ROOK) {
        // Define directional deltas for each direction (up, down, left, right).
        int xDeltas[] = {0, 0, -1, 1};
        int yDeltas[] = {-1, 1, 0, 0};

        // Loop through each direction (up, down, left, right)
        for (int dir = 0; dir < 4; ++dir) {
            int xIt = x + xDeltas[dir];
            int yIt = y + yDeltas[dir];

            // Check each square along the direction until edge of board or another piece is encountered.
            while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
                uint8_t pos_to_check = make_pos(xIt, yIt);

                // Toggle bit on attack_board and break if piece found.
                toggle_bit_on(attack_board, pos_to_check);
                if (get_piece(pos_to_check) != 0)
                    break;

                // Move to the next square in the current direction.
                xIt += xDeltas[dir];
                yIt += yDeltas[dir];
            }
        }
    }
    return attack_board;
}

// Create moving board for specific piece. This is where de define the movement logic of each piece.
uint64_t Position::make_move_board(uint8_t x, uint8_t y)
{
    uint64_t move_board = 0b0;

    int pos = make_pos(x, y);

    uint8_t piece_type = this->get_piece(pos);

    int diagonals[6][2] = {
        {x + 1, y + 1},
        {x - 1, y + 1},
        {x + 1, y - 1},
        {x - 1, y - 1},
    };

    int perpendicular[6][2] = {
        {x - 1, y},
        {x + 1, y},
        {x, y - 1},
        {x, y + 1}
    };

    // This is where we determine where a piece can go.
    if(piece_type == B_PAWN) 
    {
        if(get_piece(make_pos(x, y+1)) == 0)
        {
            if(y == 1 && get_piece(make_pos(x, y+2)) == 0)
            {
                // Pawn can move 2 squares.
                toggle_bit_on(move_board, make_pos(std::min(7, x + 0), std::min(7, y+2)));
            }
            toggle_bit_on(move_board, make_pos(std::min(7, x + 0), std::min(7, y+1)));
        }
    }
    else if(piece_type == W_PAWN)
    {
        if(get_piece(make_pos(x, y-1)) == 0)
        {
            if(y == 6 && get_piece(make_pos(x, y-2)) == 0)
            {
                // Pawn can move 2 squares.
                toggle_bit_on(move_board, make_pos(std::min(7, x + 0), std::min(7, y-2)));
            }
            toggle_bit_on(move_board, make_pos(std::min(7, x + 0), std::min(7, y-1)));
        }
    }
    else if(piece_type == B_KING || piece_type == W_KING)
    {
        int offsets[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
        };
        for (int i = 0; i < 8; ++i) {
            int new_x = x + offsets[i][0];
            int new_y = y + offsets[i][1];
            
            // Check if the new position is within bounds
            if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
                if(this->get_piece(make_pos(new_x, new_y)) == 0)
                    toggle_bit_on(move_board, make_pos(new_x, new_y));
            }
        }
    }
    else if (piece_type == B_BISHOP || piece_type == W_BISHOP || piece_type == W_QUEEN || piece_type == B_QUEEN) 
    {
        // Define directional deltas for each diagonal direction.
        int xDeltas[] = {-1, 1, -1, 1};
        int yDeltas[] = {-1, -1, 1, 1};

        // Loop through each diagonal direction.
        for (int dir = 0; dir < 4; ++dir) {
            int xIt = x + xDeltas[dir];
            int yIt = y + yDeltas[dir];

            // Check each square along the diagonal until edge of board or another piece is encountered.
            while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
                uint8_t pos_to_check = make_pos(xIt, yIt);

                if (get_piece(pos_to_check) == 0)
                    toggle_bit_on(move_board, pos_to_check);

                if (get_piece(pos_to_check) != 0)
                    break;

                // Move to the next square in the diagonal.
                xIt += xDeltas[dir];
                yIt += yDeltas[dir];
            }
        }
    }
    else if (piece_type == B_KNIGHT || piece_type == W_KNIGHT)
    {
        int offsets[8][2] = 
        {
            {1, 2}, {-1, 2}, {1, -2}, {-1, -2},
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}
        };
        for (int i = 0; i < 8; ++i) 
        {
            int new_x = x + offsets[i][0];
            int new_y = y + offsets[i][1];
            
            // Check if the new position is within bounds
            if (new_x >= 0 && new_x <= 7 && new_y >= 0 && new_y <= 7) 
            {
                uint8_t new_pos = make_pos(new_x, new_y);
                uint8_t piece_to_check = get_piece(new_pos);
                if(this->get_piece(make_pos(new_x, new_y)) == 0)
                    toggle_bit_on(move_board, new_pos);
            }
        }
    }
    if (piece_type == W_QUEEN || piece_type == B_QUEEN || piece_type == W_ROOK || piece_type == B_ROOK) {
        // Define directional deltas for each direction (up, down, left, right).
        int xDeltas[] = {0, 0, -1, 1};
        int yDeltas[] = {-1, 1, 0, 0};

        // Loop through each direction (up, down, left, right).
        for (int dir = 0; dir < 4; ++dir) {
            int xIt = x + xDeltas[dir];
            int yIt = y + yDeltas[dir];

            // Check each square along the direction until edge of board or another piece is encountered.
            while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
                uint8_t pos_to_check = make_pos(xIt, yIt);

                if (get_piece(pos_to_check) == 0)
                    toggle_bit_on(move_board, pos_to_check);

                if (get_piece(pos_to_check) != 0)
                    break;

                // Move to the next square in the current direction.
                xIt += xDeltas[dir];
                yIt += yDeltas[dir];
            }
        }
    }
    return move_board;
}

// Get position of a piece. (first found).
uint8_t Position::get_piece_position(uint8_t piece)
{
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            uint8_t piece_at_pos = get_piece(make_pos(x,y));
            if(piece_at_pos == piece)
            {
                return make_pos(x, y);
            }
        }
    }
    
    return -1;
}

// Create the attack board for all pieces of a player.
uint64_t Position::color_reach_board(bool is_white)
{
    bool color_sign = !is_white;
    

    uint64_t attack_board = 0;

    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            uint8_t piece = get_piece(make_pos(x, y));
            bool piece_color = get_color(piece);
            if(piece != 0 && color_sign == piece_color)
            {
                uint64_t piece_reach = this->make_reach_board(x, y);
                attack_board |= piece_reach;
                // print_binary(piece_reach);
                // print_binary(attack_board);
            }
        }
    }
    // print_binary(attack_board);
    return attack_board;
}

// Check if the king is in check.
bool Position::king_under_attack(bool is_white)
{
    // King is under attack if it intersects with the attack board of opposite player.
    uint8_t pos = (is_white) ? get_piece_position(W_KING) : get_piece_position(B_KING);

    // Check attack board for other position.
    uint64_t attacked_positions = color_reach_board(!is_white);

    // Check if intersect on king position.
    return get_bit_64(attacked_positions, pos);
}

void Position::set_piece(uint8_t new_piece, uint8_t pos)
{
    // This will be the new 64 bits replacing the old rows.
    uint64_t new_squares = 0b0;

    // The amount to shift in the rows.
    int shift_amount = 0;

    // The old 64 bits.
    uint64_t* old_squares = nullptr;

    // Determine the shift amount and old 64 bits.
    if (pos < 16)
    {
        shift_amount = (15-pos)*4;
        old_squares = &this->first_16;
    }
    else if (pos < 32)
    {
        shift_amount = (15 - (pos - 16))*4;
        old_squares = &this->second_16;
    }
    else if (pos < 48)
    {
        shift_amount = (15 - (pos - 32))*4;
        old_squares = &this->third_16;
    }
    else
    {
        shift_amount = (15 - (pos - 48))*4;
        old_squares = &this->fourth_16;
    }

    std::cout << shift_amount << '\n';
    // Isolate the left side of the piece to be replaced.
    uint64_t mask_left = *old_squares;
    mask_left  >>= (shift_amount + 4);
    mask_left <<= (shift_amount + 4);
    
    // Make mask for the piece.
    uint64_t mask_piece = static_cast<uint64_t>(new_piece) << shift_amount;
    // Isolate right side.
    uint64_t mask_right = *old_squares;
    mask_right <<= (64-shift_amount);
    mask_right >>= (64-shift_amount);

    if(shift_amount == 0)
        mask_right = 0ul;
    if(shift_amount == 60)
        mask_left = 0ul;

    // Make new 64 bits representing the two rows.
    new_squares |= mask_left;
    new_squares |= mask_piece;
    new_squares |= mask_right; 

    // Replace.
    *old_squares = new_squares;
}

void Position::do_move(Move* move)
{
    this->set_piece(this->get_piece(move->start_location), move->end_location);
    this->set_piece(0b0000, move->start_location);

    last_move = new Move(move->start_location, move->end_location);
}

std::vector<Move> Board::determine_moves(bool is_white, Position* position) const
{
    // Determine the color sign of the player at turn.
    uint8_t color_sign = (is_white) ? 0 : 1;

    std::vector<Move> possible_moves;

    // Loop through board to find player's pieces.
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            // Get piece at position.
            int pos = make_pos(x, y);
            uint8_t piece_type = position->get_piece(pos);

            // Empty square or wrong color.
            if(piece_type == 0 || get_color(piece_type) != color_sign) continue;

            uint64_t move_squares = position->make_move_board(x, y);
            uint64_t reach_squares = position->make_reach_board(x, y);
            
            for(int i = 0; i < 63; i++)
            {
                // Check if the piece can be moved to this square.
                uint8_t piece_at_square = position->get_piece(i);
                bool can_move = get_bit_64(move_squares, i);
                bool can_attack = false;
                bool piece_on_attack_square = get_bit_64(reach_squares, i);
                bool piece_is_other_color = color_sign != get_color(piece_at_square);
                if(piece_at_square != 0)
                    can_attack = (piece_is_other_color && piece_on_attack_square);


                if(piece_is_other_color)
                    std::cout << "piece is other color \n";
                if(piece_on_attack_square)
                    std::cout << "piece is on attacked square \n";

                if(can_move || can_attack)
                {
                    // piece at pos can move to square i.
                    Move move(pos,i);

                    // Copy board state.
                    uint64_t first = position->first_16;
                    uint64_t second = position->second_16;
                    uint64_t third = position->third_16;
                    uint64_t fourth = position->fourth_16;

                    position->do_move(&move);
                    if(!position->king_under_attack(is_white))
                    {   
                        // Move is possible.
                        possible_moves.push_back(move);
                    }

                    // Restore position.
                    position->first_16 = first;
                    position->second_16 = second;
                    position->third_16 = third;
                    position->fourth_16 = fourth;
                }
            }
        }
    }
    return possible_moves;
}

// Get the piece on position x, y.
uint8_t Position::get_piece(uint8_t pos) const
{
    // Rightshift the corresponding 64 bits to get the 4 bits of the position.
    uint8_t piece;
    if (pos < 16)
    {
        piece = this->first_16 >> (15 - pos)*4;
    }
    else if (pos < 32)
    {
        piece = this->second_16 >> (15 - (pos - 16))*4;
    }
    else if (pos < 48)
    {
        piece = this->third_16 >> (15 - (pos - 32))*4;
    }
    else
    {
        piece = this->fourth_16 >> (15 - (pos - 48))*4;
    }

    // Take only the right 4 bits.
    return (piece & 0b00001111);
}