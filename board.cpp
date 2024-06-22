#include "board.hpp"

Move::Move()
{

}

Move::Move(const Move& other)
{
    // Copy contents.
    this->start_location = other.start_location;
    this->end_location = other.end_location;
    this->special_cases = other.special_cases;
    this->move_takes_an_passant = other.move_takes_an_passant;
    this->evaluation = other.evaluation;
}

void Position::initialize()
{
    // Initialize to default chess position.
    this->first_16 = FIRST_16_SQUARES;
    this->second_16 = SECOND_16_SQUARES;
    this->third_16 = THIRD_16_SQUARES;
    this->fourth_16 = FOURTH_16_SQUARES;
}

Position::Position() {}

Position::Position(const Position& other) 
{
    // Copy contents of position.
    this->first_16 = other.first_16;
    this->second_16 = other.second_16;
    this->third_16 = other.third_16;
    this->fourth_16 = other.fourth_16;
    this->casling_rights = other.casling_rights;
    this->en_passant = other.en_passant;
}

Board::Board()
{
    position = new Position();
    position->initialize();
}

Board::~Board()
{
    // Delete the position.
    delete position;
    position = nullptr;
}

Position::~Position()
{
}

// Create attack board for specific piece. This is where we define the attacking logic for each piece.
uint64_t Position::make_reach_board(uint8_t x, uint8_t y) {
    uint64_t attack_board = 0b0;
    int pos = make_pos(x, y);
    uint8_t piece_type = this->get_piece(pos);

    switch (piece_type) {
        case B_PAWN:
            attack_board = get_pawn_reach(x, y, false);
            break;
        case W_PAWN:
            attack_board = get_pawn_reach(x, y, true);
            break;
        case B_KING:
        case W_KING:
            attack_board = get_king_reach(x, y);
            break;
        case B_BISHOP:
        case W_BISHOP:
            attack_board = get_bishop_reach(x, y);
            break;
        case B_KNIGHT:
        case W_KNIGHT:
            attack_board = get_knight_reach(x, y);
            break;
        case B_ROOK:
        case W_ROOK:
            attack_board = get_rook_reach(x, y);
            break;
        case B_QUEEN:
        case W_QUEEN:
            attack_board = get_queen_reach(x, y);
            break;
        default:
            break;
    }

    return attack_board;
}

// Create moving board for specific piece. This is where de define the movement logic of each piece.
uint64_t Position::make_move_board(uint8_t x, uint8_t y) {
    uint64_t move_board = 0b0;
    int pos = make_pos(x, y);
    uint8_t piece_type = this->get_piece(pos);

    switch (piece_type) {
        case B_PAWN:
            move_board = get_pawn_move(x, y, false);
            break;
        case W_PAWN:
            move_board = get_pawn_move(x, y, true);
            break;
        case B_KING:
        case W_KING:
            move_board = get_king_move(x, y);
            break;
        case B_BISHOP:
        case W_BISHOP:
            move_board = get_bishop_move(x, y);
            break;
        case B_KNIGHT:
        case W_KNIGHT:
            move_board = get_knight_move(x, y);
            break;
        case B_ROOK:
        case W_ROOK:
            move_board = get_rook_move(x, y);
            break;
        case B_QUEEN:
        case W_QUEEN:
            move_board = get_queen_move(x, y);
            break;
        default:
            break;
    }

    return move_board;
}

// Get position of a piece. (first one of its type found).
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
uint64_t Position::color_reach_board(bool color_sign)
{
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
            }
        }
    }
    // print_binary(attack_board);
    return attack_board;
}

// Check if the king is in check.
bool Position::king_under_attack(bool color_sign)
{
    // King is under attack if it intersects with the attack board of opposite player.
    uint8_t pos = (color_sign) ? get_piece_position(B_KING) : get_piece_position(W_KING);

    // Check attack board for other position.
    uint64_t attacked_positions = color_reach_board(!color_sign);

    // Check if intersect on king position.
    return get_bit_64(attacked_positions, pos);
}

void Position::set_piece(uint8_t new_piece, uint8_t pos)
{
    assert(pos <= 63);
    assert(pos >= 0);
    
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
    if(!move->move_bounds_valid())
    {
        std::cout << "Invalid move found! \n"; 
    }
    assert(move->start_location >= 0 && move->start_location <= 63);
    assert(move->end_location >= 0 && move->end_location <= 63);
    move_piece(move);
    handle_en_passant_capture(move);
    reset_en_passant_status();
    handle_special_cases(move);
}

void Position::move_piece(Move* move)
{
    this->set_piece(this->get_piece(move->start_location), move->end_location);
    this->set_piece(EMPTY, move->start_location);
}

// TODO: fix crash.
void Position::handle_en_passant_capture(Move* move)
{
    // Check if move was an en passant capture.
    if (move->move_takes_an_passant)
    {
        uint8_t taking_pawn_color = get_piece(move->end_location);

        if (taking_pawn_color == B_PAWN && move->end_location >= 8)
        {
            // Black pawn captured en passant (from row 3 to row 2).
            set_piece(EMPTY, move->end_location - 8);
        }
        else if (taking_pawn_color == W_PAWN && move->end_location < 56)
        {
            // White pawn captured en passant (from row 4 to row 5).
            set_piece(EMPTY, move->end_location + 8);
        }
    }
}

void Position::reset_en_passant_status()
{
    // Reset an passant status.
    en_passant = 0b11111111;
}

void Position::handle_special_cases(Move* move)
{
    // Check if move is a special case.
    if(move->special_cases != 0 && move->special_cases != 5)
    {
        handle_castling(move);
    }
    // Check if an passant is possible after this move.
    else if(move->special_cases == 5)
    {
        check_en_passant_possibility(move);
    }
}

void Position::handle_castling(Move* move)
{
    // Check which rook to move.
    uint8_t rook_start, rook_end;
    switch(move->special_cases)
    {
        case 1: // White kingside
            rook_start = 63;
            rook_end = 61;
            break;
        case 2: // White queenside
            rook_start = 56;
            rook_end = 59;
            break;
        case 3: // Black kingside
            rook_start = 7;
            rook_end = 5;
            break;
        case 4: // Black queenside
            rook_start = 0;
            rook_end = 2;
            break;
        default:
            return;
    }
    this->set_piece(this->get_piece(rook_start), rook_end);
    this->set_piece(EMPTY, rook_start);
}

void Position::check_en_passant_possibility(Move* move)
{
    // Move is a pawn 2 forward, check if an passant is possible.
    uint8_t piece_on_left = (move->end_location % 8 > 0) ? get_piece(move->end_location - 1) : EMPTY;
    uint8_t piece_on_right = (move->end_location % 8 < 7) ? get_piece(move->end_location + 1) : EMPTY;
    uint8_t color_sign = get_color(get_piece(move->end_location));

    if(piece_on_left == W_PAWN || piece_on_left == B_PAWN)
    {
        if(color_sign != get_color(piece_on_left))
        {
            // An passant is possible for pawn on -1.
            this->en_passant = 0b10000000;
            this->en_passant += (move->end_location % 8);
            if(color_sign)
                en_passant |= 0b01000000;
        }
    }
    else if(piece_on_right == W_PAWN || piece_on_right == B_PAWN)
    {
        if(color_sign != get_color(piece_on_right))
        {
            // An passant is possible for pawn on +1.
            this->en_passant = 0b00000000;
            this->en_passant += (move->end_location % 8);
            if(color_sign)
                en_passant |= 0b01000000;
        }
    }
}

std::vector<Move> Position::determine_moves(bool color_sign)
{
    std::vector<Move> possible_moves;

    // Loop through board to find player's pieces.
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            int pos = make_pos(x, y);
            uint8_t piece_type = this->get_piece(pos);

            // Empty square or wrong color.
            if (piece_type == 0 || get_color(piece_type) != color_sign)
                continue;

            uint64_t move_squares = this->make_move_board(x, y);
            uint64_t reach_squares = this->make_reach_board(x, y);

            // Generate moves for the piece.
            generate_piece_moves(pos, piece_type, move_squares, reach_squares, color_sign, possible_moves);
        }
    }

    // Check castling rights.
    if(!king_under_attack(color_sign))
        generate_castling_moves(color_sign, possible_moves);

    // Check en passant.
    generate_en_passant_move(color_sign, possible_moves);

    return possible_moves;
}

void Position::generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, uint64_t reach_squares, bool color_sign, std::vector<Move>& possible_moves)
{
    for (int i = 0; i < 64; i++)
    {
        uint8_t piece_at_square = this->get_piece(i);
        bool can_move = get_bit_64(move_squares, i);
        bool can_attack = (piece_at_square != 0) && (color_sign != get_color(piece_at_square)) && get_bit_64(reach_squares, i);

        if (can_move || can_attack)
        {
            Move move(pos, i);

            // Simulate the move.
            Position* copy = new Position(*this);
            copy->do_move(&move);

            // Check if king is not under attack after the move.
            if (!copy->king_under_attack(color_sign))
            {
                // Handle special case for pawn two-square move.
                if ((piece_type == B_PAWN || piece_type == W_PAWN) && (i - pos) > 8)
                {
                    move.special_cases = 5;
                }
                if(move.move_bounds_valid())
                    possible_moves.push_back(move);
            }

            delete copy;
        }
    }
}

// TODO: check if castling is not under check.
void Position::generate_castling_moves(bool color_sign, std::vector<Move>& possible_moves)
{
    if (color_sign && get_bit(casling_rights, 6))
    {
        // Black kingside castling.
        if (get_piece(5) == 0 && get_piece(6) == 0)
        {
            Move move(4, 6);
            move.special_cases = 3;
            possible_moves.push_back(move);
        }
    }

    if (color_sign && get_bit(casling_rights, 7))
    {
        // Black queenside castling.
        if (get_piece(1) == 0 && get_piece(2) == 0 && get_piece(3) == 0)
        {
            Move move(4, 2);
            move.special_cases = 4;
            possible_moves.push_back(move);
        }
    }

    if (!color_sign && get_bit(casling_rights, 4))
    {
        // White kingside castling.
        if (get_piece(61) == 0 && get_piece(62) == 0)
        {
            Move move(60, 62);
            move.special_cases = 1;
            possible_moves.push_back(move);
        }
    }

    if (!color_sign && get_bit(casling_rights, 5))
    {
        // White queenside castling.
        if (get_piece(59) == 0 && get_piece(58) == 0 && get_piece(57) == 0)
        {
            Move move(60, 58);
            move.special_cases = 2;
            possible_moves.push_back(move);
        }
    }
}

// TODO: check if not in check afterwards.
void Position::generate_en_passant_move(bool color_sign, std::vector<Move>& possible_moves)
{
    if (en_passant != 0b11111111)
    {
        uint8_t to = (en_passant & 0b00111111);
        uint8_t from = to + ((en_passant & 0b10000000) ? -1 : +1);
        uint8_t row = color_sign ? 3 : 4;

        uint8_t end_square = make_pos(to, row - 1);
        uint8_t start_square = make_pos(from, row);

        Move move(start_square, end_square);
        move.move_takes_an_passant = true;
        if(move.move_bounds_valid())
            possible_moves.push_back(move);
    }
}

bool Move::move_bounds_valid()
{
    return  (this->start_location >= 0 && this->start_location <= 63)
            && (this->end_location >= 0 && this->end_location <= 63);
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

bool Move::is_check(Position* position) const
{
    Position* copy = new Position(*position);
    Move copy_move(start_location, end_location);
    if(copy_move.move_bounds_valid())
        copy->do_move(&copy_move);

    bool is_check = copy->king_under_attack(!get_color(position->get_piece(this->start_location)));

    delete copy;

    return is_check;
}

bool Move::is_capture(Position* position) const
{
    uint8_t start_square = position->get_piece(this->start_location);
    uint8_t capture_square = position->get_piece(this->end_location);
    if(capture_square != 0 && get_color(start_square) != get_color(end_location))
        return true;
    return false;
}

float Move::capture_value(Position* position) const
{
    uint8_t capture_square = position->get_piece(this->end_location);
    return get_piece_value(capture_square);
}