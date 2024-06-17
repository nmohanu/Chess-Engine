#include "board.hpp"

void Position::initialize()
{
    this->first_16 = FIRST_16_SQUARES;
    this->second_16 = SECOND_16_SQUARES;
    this->third_16 = THIRD_16_SQUARES;
    this->fourth_16 = FOURTH_16_SQUARES;
}

Board::Board()
{
    position->initialize();
}

Board::~Board()
{
    delete position;
}

// Create attack board for specific piece.
uint64_t Position::make_attack_board(uint8_t x, uint8_t y)
{
    uint64_t attack_board = 0b0;

    int pos = make_pos(x, y);

    uint8_t piece_type = this->get_piece(pos);

    if(piece_type == B_PAWN) 
    {
        toggle_bit_on(attack_board, make_pos(x + 1, y + 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y + 1));
    }
    else if(piece_type == W_PAWN)
    {
        toggle_bit_on(attack_board, make_pos(x + 1, y - 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y - 1));
    }
    else if(piece_type == B_KING || piece_type == W_KING)
    {
        toggle_bit_on(attack_board, make_pos(x + 1, y - 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y - 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y + 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y + 1));
        toggle_bit_on(attack_board, make_pos(x - 1, y));
        toggle_bit_on(attack_board, make_pos(x + 1, y));
        toggle_bit_on(attack_board, make_pos(x, y - 1));
        toggle_bit_on(attack_board, make_pos(x, y + 1));
    }
    else if(piece_type == B_BISHOP || piece_type == W_BISHOP || piece_type == W_QUEEN || piece_type == B_QUEEN) 
    {
        // Set diagonals to 1 untill another piece is found.
        int xIt = x-1;
        int yIt = y-1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            uint8_t piece_on_pos = get_piece(pos_to_check);
            if(piece_on_pos != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            xIt--;
            yIt--;
        }

        xIt = x+1;
        yIt = y-1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            xIt++;
            yIt--;
        }

        xIt = x-1;
        yIt = y+1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            xIt--;
            yIt++;
        }

        xIt = x+1;
        yIt = y-1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            xIt++;
            yIt--;
        }
    }
    else if (piece_type == B_KNIGHT || piece_type == W_KNIGHT)
    {
        toggle_bit_on(attack_board, make_pos(x + 1, y + 2));
        toggle_bit_on(attack_board, make_pos(x - 1, y + 2));
        toggle_bit_on(attack_board, make_pos(x + 1, y - 2));
        toggle_bit_on(attack_board, make_pos(x - 1, y - 2));
        toggle_bit_on(attack_board, make_pos(x + 2, y + 1));
        toggle_bit_on(attack_board, make_pos(x + 2, y - 2));
        toggle_bit_on(attack_board, make_pos(x - 2, y + 1));
        toggle_bit_on(attack_board, make_pos(x - 2, y - 2));
    }
    if (piece_type == W_QUEEN || piece_type == B_QUEEN || piece_type == W_ROOK || piece_type == B_ROOK)
    {
        int xIt = x;
        int yIt = y-1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            yIt--;
        }
        xIt = x;
        yIt = y+1;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            yIt++;
        }
        xIt = x-1;
        yIt = y;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            xIt--;
        }
        xIt = x+1;
        yIt = y;
        while(true && xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8)
        {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            if(get_piece(pos_to_check) != 0)
            {
                toggle_bit_on(attack_board, pos_to_check);
                break;
            }
            toggle_bit_on(attack_board, pos_to_check);
            yIt++;
        }
    }
    return attack_board;
}

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
uint64_t Position::color_attack_board(bool is_white)
{
    uint8_t color_sign = !is_white;

    uint64_t attack_board = 0;

    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            if(color_sign == get_color(get_piece(make_pos(x, y))))
                attack_board |= this->make_attack_board(x, y);
        }
    }
}

bool Position::king_under_attack(bool is_white)
{
    // King is under attack if it intersects with the attack board of opposite player.
    uint8_t pos = (is_white) ? get_piece_position(W_KING) : get_piece_position(B_KING);

    // Check attack board for other position.
    uint64_t attacked_positions = color_attack_board(!is_white);

    // Check if intersect on king position.
    if(get_bit(attacked_positions, pos))
    {
        return true;
    }
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

    // Isolate the left side of the piece to be replaced.
    uint64_t mask_left = (*old_squares >> (shift_amount + 4) << (shift_amount + 4));
    // Make mask for the piece.
    uint64_t mask_piece = new_piece << shift_amount;
    // Isolate right side.
    uint64_t mask_right = (*old_squares << (shift_amount - 4) >> (shift_amount - 4));

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
    this->set_piece(0, move->start_location);
}

std::vector<Move*> Board::determine_moves(bool is_white, Position &position) const
{
    uint8_t color_sign = (is_white) ? 0 : 1;
    std::vector<Move*> possible_moves;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            // Get piece at position.
            int pos = make_pos(x, y);
            uint8_t piece_type = position.get_piece(pos);

            // Empty square.
            if(piece_type == 0 || get_color(piece_type) != color_sign) continue;

            uint64_t attacksquares = position.make_attack_board(x, y);
            
            for(int i = 0; i < 63; i++)
            {
                if(get_bit_64(attacksquares, i) == 1)
                {
                    // piece at pos can move to square i.
                    Move* move = new Move(pos,i);

                    // Copy board state.
                    uint64_t first = position.first_16;
                    uint64_t second = position.second_16;
                    uint64_t third = position.third_16;
                    uint64_t fourth = position.fourth_16;

                    position.do_move(move);
                    if(!position.king_under_attack(is_white))
                    {   
                        // Move is possible.
                        possible_moves.push_back(move);
                    }
                    // Restore position.
                    position.first_16 = first;
                    position.second_16 = second;
                    position.third_16 = third;
                    position.fourth_16 = fourth;
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


void Board::print() const
{
    // for(int i = 0; i < 8; i++)
    // {
    //     std::cout<< std::setw(5) << "=====";
    // }
    // std::cout << std::endl;
        
    // for(int y = 0; y < 8; y++)
    // {
    //     for(int x = 0; x < 8; x++)
    //     {
    //         int pos = y * 8 + x;
    //         if(get_bit(position->pawns, pos))
    //         {
    //             std::cout << std::setw(5) << "| P |";
    //         }
    //         else if(get_bit(position->bishops, pos))
    //         {
    //             std::cout << std::setw(5) << "| B |";
    //         }
    //         else if(get_bit(position->rooks, pos))
    //         {
    //             std::cout << std::setw(5) << "| R |";
    //         }
    //         else if(get_bit(position->knights, pos))
    //         {
    //             std::cout << std::setw(5) << "| H |";
    //         }
    //         else if(get_bit(position->kings, pos))
    //         {
    //             std::cout << std::setw(5) << "| K |";
    //         }
    //         else if(get_bit(position->queens, pos))
    //         {
    //             std::cout << std::setw(5) << "| Q |";
    //         }
    //         else
    //         {
    //             std::cout<< std::setw(5) << "|   |";
    //         }
    //     }
    //     std::cout << std::endl;
    //     for(int i = 0; i < 8; i++)
    //     {
    //         std::cout<< std::setw(5) << "=====";
    //     }
    //     std::cout << std::endl;
    // }
}