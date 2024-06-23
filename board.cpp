#include "board.hpp"


// ==============================================================================================

// Move constructors.
Move::Move() {}

Move::Move(const Move& other)
{
    // Copy contents.
    this->start_location = other.start_location;
    this->end_location = other.end_location;
    this->special_cases = other.special_cases;
    this->move_takes_an_passant = other.move_takes_an_passant;
    this->evaluation = other.evaluation;
    this->moving_piece = other.moving_piece;
    this->captured_piece = other.captured_piece;
}

// ==============================================================================================

// Position constructors.
Position::Position() {}

Position::Position(const Position& other) 
{
    // Copy contents of position.
    this->casling_rights = other.casling_rights;
    this->en_passant = other.en_passant;
    this->white_to_turn = other.white_to_turn;

    // Copy bitboards.
    for(uint8_t piece = W_KING; piece < B_PAWN; piece++)
    {
        this->bit_boards[piece] = other.bit_boards[piece];
    }
}

// Destructor.
Position::~Position() {}

// ==============================================================================================

// Board constructors.
Board::Board()
{
    position = new Position();
}

Board::~Board()
{
    // Delete the position.
    delete position;
    position = nullptr;
}

// ==============================================================================================

// Create attack board for specific piece. This is where we define the attacking logic for each piece.
uint64_t Position::make_reach_board(uint8_t square, bool is_black) 
{
    uint64_t attack_board = 0b0;
    uint8_t piece_type = this->get_piece(square);

    switch (piece_type) {
        case B_PAWN:
        case W_PAWN:
            attack_board = get_pawn_move(square, is_black);
            break;
        case B_KING:
        case W_KING:
            attack_board = get_king_move(square, is_black);
            break;
        case B_BISHOP:
        case W_BISHOP:
            attack_board = get_bishop_move(square, is_black);
            break;
        case B_KNIGHT:
        case W_KNIGHT:
            attack_board = get_knight_move(square, is_black);
            break;
        case B_ROOK:
        case W_ROOK:
            attack_board = get_rook_move(square, is_black);
            break;
        case B_QUEEN:
        case W_QUEEN:
            attack_board = get_queen_move(square, is_black);
            break;
        default:
            attack_board = 0b0;
            break;
    }

    return attack_board;
}

// ==============================================================================================

// Create the attack board for all pieces of a player.
uint64_t Position::color_reach_board(bool is_black)
{
    uint64_t attack_board = 0;

    for(uint8_t square = 0; square < 64; square++)
    {
        // Check if piece belongs to color we are checking. Also skip if square is empty.
        if(is_black != get_bit_64(bit_boards[BLACK_PIECES], square) || !get_bit_64(bit_boards[TOTAL], square))
            continue;
        
        // Add piece reach to total reach.
        attack_board |= make_reach_board(square, is_black);
    }
    // print_binary(attack_board);
    return attack_board;
}

// ==============================================================================================

// Check if the king is in check.
bool Position::king_under_attack(bool is_black)
{
    // King is under attack if it intersects with the attack board of opposite player.
    uint64_t king_board = (is_black) ? bit_boards[B_KING] : bit_boards[W_KING];

    // Check attack board for other position.
    uint64_t attacked_positions = color_reach_board(!is_black);

    // Check if intersect on king position.
    return (king_board & attacked_positions) > 0;
}

// ==============================================================================================

void Position::do_move(Move* move)
{
    if(!move->move_takes_an_passant)
        move_piece(move);
    else
        handle_en_passant_capture(move);
    reset_en_passant_status();
    handle_special_cases(move);
}

// ==============================================================================================

void Position::move_piece(Move* move)
{
    // Double check. Should be in bounds, but this way we can catch bugs correlating to 
    // memory corruption, for example.
    uint8_t start_square = move->start_location;
    uint8_t end_square = move->end_location;
    assert(square_in_bounds(start_square) && square_in_bounds(end_square));
    
    // The piece we are moving.
    uint8_t moved_piece = move->moving_piece;

    // Toggle bit in piece board.
    toggle_bit_on(bit_boards[moved_piece], end_square);
    toggle_bit_off(bit_boards[moved_piece], start_square);

    // Toggle bit in total board.
    toggle_bit_on(bit_boards[TOTAL], end_square);
    toggle_bit_off(bit_boards[TOTAL], start_square);

    // If necessary, toggle bit in color board.
    if(moved_piece > 5)
    {
        toggle_bit_on(bit_boards[BLACK_PIECES], end_square);
        toggle_bit_off(bit_boards[BLACK_PIECES], start_square);
    }

    // Check if there was a piece captured.
    uint8_t captured_piece = move->captured_piece;
    if(captured_piece < 12)
    {
        toggle_bit_off(bit_boards[captured_piece], end_square);

        // Now check if we need to update color board because of capture.
        if(captured_piece > 5)
        {
            toggle_bit_off(bit_boards[BLACK_PIECES], end_square);
        }
    }
}

// ==============================================================================================

// Process en passant capture seperately.
void Position::handle_en_passant_capture(Move* move)
{
    // Check if move was an en passant capture.
    if (move->move_takes_an_passant)
    {
        uint8_t start_location = move->start_location;
        uint8_t end_location = move->end_location;
        bool taking_pawn_black = move->moving_piece > 5;

        // Move pawn.
        toggle_bit_off(bit_boards[W_PAWN + 6*taking_pawn_black], start_location);
        // Pawn is captured. Toggle bits off for taken piece.
        toggle_bit_off(bit_boards[W_PAWN + 6*taking_pawn_black], ((end_location + 8) - 16*taking_pawn_black));
        // Total board taken piece.
        toggle_bit_off(bit_boards[TOTAL], ((end_location + 8) - 16*taking_pawn_black));
        // Total board new position.
        toggle_bit_on(bit_boards[TOTAL], end_location);
        // Update color board.
        if(taking_pawn_black)
        {
            toggle_bit_on(bit_boards[BLACK_PIECES], end_location);
            toggle_bit_off(bit_boards[BLACK_PIECES], start_location);
        }
        else
        {
            toggle_bit_off(bit_boards[BLACK_PIECES], end_location + 8);
        }
    }
}

// ==============================================================================================

// After a move is done, we need to reset the en passant status to default.
// After the move is done, we check to see if en passant is possible and update accordingly.
void Position::reset_en_passant_status()
{
    // Reset an passant status.
    en_passant = 0b11111111;
}

// ==============================================================================================

// Check if we need to handle castling or update en passant rights.
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

// ==============================================================================================

// Handle castling move.
void Position::handle_castling(Move* move)
{
    bool is_black = move->moving_piece > 5;

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
    // Move rook and update bitboards.
    toggle_bit_off(bit_boards[W_ROOK + 6*is_black], rook_start);
    toggle_bit_on(bit_boards[W_ROOK + 6*is_black], rook_end);
    if(is_black)
    {
        toggle_bit_off(bit_boards[BLACK_PIECES], rook_start);
        toggle_bit_off(bit_boards[BLACK_PIECES], rook_end);
    }
    toggle_bit_off(bit_boards[TOTAL], rook_start);
    toggle_bit_off(bit_boards[TOTAL], rook_end);
}

// ==============================================================================================

// Check en passant possibility and update rights.
void Position::check_en_passant_possibility(Move* move)
{
    // Move is a pawn 2 forward, check if an passant is possible.
    uint8_t piece_on_left = (move->end_location % 8 > 0) ? get_piece(move->end_location - 1) : EMPTY;
    uint8_t piece_on_right = (move->end_location % 8 < 7) ? get_piece(move->end_location + 1) : EMPTY;
    bool is_black = move->moving_piece > 5;

    // Check if there is an enemy pawn next to the moved pawn.
    if(piece_on_left - is_black*6 == W_PAWN && is_black != (piece_on_left > 5))
    {
        // An passant is possible for pawn on -1.
        this->en_passant = 0b10000000;
        this->en_passant += (move->end_location % 8);
        if(is_black)
            en_passant |= 0b01000000;
    }
    else if(piece_on_right - is_black * 6 == W_PAWN && is_black != (piece_on_left > 5))
    {
        // An passant is possible for pawn on +1.
        this->en_passant = 0b00000000;
        this->en_passant += (move->end_location % 8);
        if(is_black)
            en_passant |= 0b01000000;
    }
}

// ==============================================================================================

// Generate all possible moves for a color and return them in a vector.
std::vector<Move> Position::determine_moves(bool is_black)
{
    std::vector<Move> possible_moves;

    // Loop through board to find player's pieces.
    for (uint8_t square = 0; square < 64; square++)
    {
        // get piece type.
        uint8_t piece_type = get_piece(square);

        // Empty square or wrong color.
        if (piece_type == EMPTY || is_black != (piece_type > 5))
            continue;

        uint64_t move_squares = make_reach_board(square, is_black);

        // Generate moves for the piece.
        generate_piece_moves(square, piece_type, move_squares, is_black, possible_moves);
    }

    // Check castling rights.
    if(!king_under_attack(is_black))
        generate_castling_moves(is_black, possible_moves);

    // Check en passant.
    generate_en_passant_move(is_black, possible_moves);

    // Return the found moves.
    return possible_moves;
}

void Position::generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, std::vector<Move>& possible_moves)
{
    for (int i = 0; i < 64; i++)
    {
        uint8_t piece_at_square = this->get_piece(i);
        bool can_move = get_bit_64(move_squares, i);
        bool can_attack = (piece_at_square != EMPTY) && (is_black != (piece_at_square > 5));

        if (can_move || can_attack)
        {
            Move move(pos, i);
            
            // Insert move data.
            if(can_attack)
                move.captured_piece = piece_at_square;
            move.moving_piece = piece_type;

            // Simulate the move.
            Position* copy = new Position(*this);
            copy->do_move(&move);

            // Check if king is not under attack after the move.
            if (!copy->king_under_attack(is_black))
            {
                // Handle special case for pawn two-square move.
                if ((piece_type == B_PAWN && (i - pos) == 16 || piece_type == W_PAWN && (pos - i) == 16))
                {
                    move.special_cases = 5;
                }
                // Insert into possible moves vector.
                if(move.move_bounds_valid())
                    possible_moves.push_back(move);
            }

            delete copy;
        }
    }
}

// ==============================================================================================

// TODO: check if castling is not under check.
// Generate all possible castling moves.
void Position::generate_castling_moves(bool is_black, std::vector<Move>& possible_moves)
{
    if (is_black && get_bit(casling_rights, 6))
    {
        // Black kingside castling.
        if (get_piece(5) == EMPTY && get_piece(6) == EMPTY)
        {
            Move move(4, 6);
            move.moving_piece = B_ROOK;
            move.special_cases = 3;
            if(move.move_bounds_valid())
                possible_moves.push_back(move);
        }
    }
    else if (!is_black && get_bit(casling_rights, 4))
    {
        // White kingside castling.
        if (get_piece(61) == EMPTY && get_piece(62) == EMPTY)
        {
            Move move(60, 62);
            move.moving_piece = W_ROOK;
            move.special_cases = 1;
            if(move.move_bounds_valid())
                possible_moves.push_back(move);
        }
    }

    if (is_black && get_bit(casling_rights, 7))
    {
        // Black queenside castling.
        if (get_piece(1) == EMPTY && get_piece(2) == EMPTY && get_piece(3) == EMPTY)
        {
            Move move(4, 2);
            move.moving_piece = B_ROOK;
            move.special_cases = 4;
            if(move.move_bounds_valid())
                possible_moves.push_back(move);
        }
    }
    else if (!is_black && get_bit(casling_rights, 5))
    {
        // White queenside castling.
        if (get_piece(59) == EMPTY && get_piece(58) == EMPTY && get_piece(57) == EMPTY)
        {
            Move move(60, 58);
            move.moving_piece = W_ROOK;
            move.special_cases = 2;
            if(move.move_bounds_valid())
                possible_moves.push_back(move);
        }
    }
}

// ==============================================================================================

// TODO: check if not in check afterwards. also double check for bugs.
// Generate en passant moves.
void Position::generate_en_passant_move(bool is_black, std::vector<Move>& possible_moves)
{
    if (en_passant != 0b11111111)
    {
        uint8_t to = (en_passant & 0b00111111);
        uint8_t from = to + ((en_passant & 0b10000000) ? -1 : +1);
        uint8_t row = is_black ? 3 : 4;

        uint8_t end_square = to + (row-1) * 8;
        uint8_t start_square = from + row * 8;

        Move move(start_square, end_square);
        move.move_takes_an_passant = true;
        if(move.move_bounds_valid())
            possible_moves.push_back(move);
    }
}

// ==============================================================================================

// Function to check if the bounds of a move are valid.
bool Move::move_bounds_valid()
{
    return  square_in_bounds(start_location) && square_in_bounds(end_location);
}

// ==============================================================================================

// Get the piece on position x, y.
uint8_t Position::get_piece(uint8_t pos) const
{   
    if(!get_bit_64(bit_boards[TOTAL], pos))
        return EMPTY;
    else if (get_bit_64(bit_boards[B_PAWN], pos))
        return B_PAWN;      // Black pawn
    else if (get_bit_64(bit_boards[W_PAWN], pos))
        return W_PAWN;      // White pawn
    else if (get_bit_64(bit_boards[B_KNIGHT], pos))
        return B_KNIGHT;    // Black knight
    else if (get_bit_64(bit_boards[W_KNIGHT], pos))
        return W_KNIGHT;    // White knight
    else if (get_bit_64(bit_boards[B_BISHOP], pos))
        return B_BISHOP;    // Black bishop
    else if (get_bit_64(bit_boards[W_BISHOP], pos))
        return W_BISHOP;    // White bishop
    else if (get_bit_64(bit_boards[B_ROOK], pos))
        return B_ROOK;      // Black rook
    else if (get_bit_64(bit_boards[W_ROOK], pos))
        return W_ROOK;      // White rook
    else if (get_bit_64(bit_boards[B_QUEEN], pos))
        return B_QUEEN;     // Black queen
    else if (get_bit_64(bit_boards[W_QUEEN], pos))
        return W_QUEEN;     // White queen
    else if (get_bit_64(bit_boards[B_KING], pos))
        return B_KING;      // Black king
    else if (get_bit_64(bit_boards[W_KING], pos))
        return W_KING;      // White king
     
    return INVALID;
}

// ==============================================================================================

// Check if a position is in check after this move is done.
bool Move::is_check(Position* position) const
{
    Position* copy = new Position(*position);
    Move copy_move(start_location, end_location);
    if(copy_move.move_bounds_valid())
        copy->do_move(&copy_move);

    bool is_check = copy->king_under_attack( !(moving_piece > 5) );

    delete copy;

    return is_check;
}

// ==============================================================================================

// Check if move is a capture.
bool Move::is_capture(Position* position) const
{
    // We need to check that the color of the start square is different than the end square.
    // Secondly, we need to check that there is a piece on the target square.
    // Otherwise, black piece to empty square would be seen as a capture.
    return (get_bit_64(position->bit_boards[BLACK_PIECES], start_location) != get_bit_64(position->bit_boards[BLACK_PIECES], end_location)) 
        && get_bit_64(position->bit_boards[TOTAL], end_location);
}

// ==============================================================================================

// Get capture value of a move.
float Move::capture_value(Position* position) const
{
    return get_piece_value(position->get_piece(this->end_location));
}