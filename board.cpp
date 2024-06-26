#include "board.hpp"

// ==============================================================================================

// Board and position logic. 

// ==============================================================================================

// Move constructors.
Move::Move() {}

Move::Move(Move* other)
{
    // Copy contents.
    this->start_location = other->start_location;
    this->end_location = other->end_location;
    this->special_cases = other->special_cases;
    this->move_takes_an_passant = other->move_takes_an_passant;
    this->evaluation = other->evaluation;
    this->moving_piece = other->moving_piece;
    this->captured_piece = other->captured_piece;
    this->previous_en_passant = other->previous_en_passant;
}

// ==============================================================================================

// Position constructors.
Position::Position() 
{
}

Position::Position(const Position& other) 
{
    // Copy contents of position.
    this->casling_rights = other.casling_rights;
    this->en_passant = other.en_passant;
    this->white_to_turn = other.white_to_turn;

    // Copy bitboards.
    for(uint8_t piece = W_KING; piece < 14; piece++)
    {
        this->bit_boards[piece] = other.bit_boards[piece];
    }
}

// Destructor.
Position::~Position() 
{   
}

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
uint64_t Position::make_reach_board(uint8_t square, bool is_black, uint8_t piece_type) 
{
    uint64_t attack_board = 0b0;

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
    uint64_t bit_mask = 1ULL << 63;

    for(uint8_t square = 0; square < 64; square++)
    {
        bool piece_at_square_black = (bit_mask & bit_boards[COLOR_BOARD]);
        // Check if piece belongs to color we are checking. Also skip if square is empty.
        if(!(bit_mask & bit_boards[TOTAL]) || (is_black != piece_at_square_black))
        {
            bit_mask >>= 1;
            continue;
        }
        
        // Add piece reach to total reach.
        attack_board |= make_reach_board(square, is_black, get_piece(square));
        bit_mask >>= 1;
    }
    // print_binary(attack_board);
    return attack_board;
}

// ==============================================================================================

// Check if the king is in check.
bool Position::king_under_attack(bool is_black, uint64_t enemy_reach)
{
    // King is under attack if it intersects with the attack board of opposite player.
    uint64_t king_board = (is_black) ? bit_boards[B_KING] : bit_boards[W_KING];

    // Check if intersect on king position.
    return boards_intersect(king_board, enemy_reach);
}

// ==============================================================================================

// Execute a move.
void Position::do_move(Move* move)
{
    move->previous_en_passant = en_passant;
    assert(move->moving_piece != INVALID);
    assert(move->moving_piece < 12);
    if(!move->move_takes_an_passant)
        move_piece(move);
    else
        handle_en_passant_capture(move);
    reset_en_passant_status();
    handle_special_cases(move);
}

// ============================================================================================== 

// Handle the undo logic for a move.
void Position::undo_move(Move* move)
{
    uint8_t start_square = move->start_location;
    uint8_t end_square = move->end_location;
    uint8_t captured_piece = move->captured_piece;
    uint8_t moved_piece = move->moving_piece;
    uint64_t end_square_mask = ~(1ULL << (63-end_square));
    uint64_t start_square_mask = 1ULL << (63-start_square);
    
    // place captured piece back on board.
    if(!move->move_takes_an_passant)
    {
        bit_boards[moved_piece] &= end_square_mask;
        bit_boards[moved_piece] |= start_square_mask;

        // Toggle bit in total board.
        bit_boards[TOTAL] &= end_square_mask;
        bit_boards[TOTAL] |= start_square_mask;

        if(moved_piece > 5)
        {
            bit_boards[COLOR_BOARD] &= end_square_mask;
            bit_boards[COLOR_BOARD] |= start_square_mask;
        }

        if(captured_piece < 12)
        {
            bit_boards[captured_piece] |= ~end_square_mask;

            // Now check if we need to update color board because of capture.
            if(captured_piece > 5)
            {
                bit_boards[COLOR_BOARD] |= ~end_square_mask;
            }
            bit_boards[TOTAL] |= ~end_square_mask;
        }
    }
    else
    {
        uint8_t capture_square = (moved_piece > 5) ? end_square - 8 : end_square + 8;
        uint8_t capture_piece_index = W_PAWN + 6 * !(moved_piece > 5);

        // Toggle off moved piece from end square
        bit_boards[moved_piece] &= end_square_mask;
        bit_boards[moved_piece] |= start_square_mask;

        bit_boards[TOTAL] &= end_square_mask;
        bit_boards[TOTAL] |= start_square_mask;

        // Restore captured pawn
        uint64_t capture_square_mask = 1ULL << (63-capture_square);
        bit_boards[capture_piece_index] |= capture_square_mask;
        bit_boards[TOTAL] |= capture_square_mask;
        if (moved_piece > 5)
        {
            bit_boards[COLOR_BOARD] &= end_square_mask;
            bit_boards[COLOR_BOARD] |= start_square_mask;
        }
        else
        {
            bit_boards[COLOR_BOARD] |= capture_square_mask;
        }
    }
    // Undo castling.
    if(move->special_cases != 0 && move->special_cases != 5)
    {
        if (move->end_location == 2)
        {   // Black queenside.
            uint64_t bit_mask = 1ULL << (63-3);
            bit_boards[B_ROOK] &= ~bit_mask;
            bit_mask <<= 3;
            bit_boards[B_ROOK] |= bit_mask;
            casling_rights |= 0b00000001;
        }
        else if (move->end_location == 6)
        {   // Black kingside.
            uint64_t bit_mask = 1ULL << (63-5);
            bit_boards[B_ROOK] &= ~bit_mask;
            bit_mask >>= 2;
            bit_boards[B_ROOK] |= bit_mask;
            casling_rights |= 0b00000010;
        }
        if (move->end_location == 58)
        {   // White queenside.
            uint64_t bit_mask = 1ULL << (63-59);
            bit_boards[W_ROOK] &= ~bit_mask;
            bit_mask <<= 3;
            bit_boards[W_ROOK] |= bit_mask;
            casling_rights |= 0b00000100;
        }
        else 
        {   // White kingside.
            uint64_t bit_mask = 1ULL << 2;
            bit_boards[W_ROOK] &= ~bit_mask;
            bit_mask >>= 2;
            bit_boards[W_ROOK] |= bit_mask;
            casling_rights |= 0b00001000;
        }
    }
    // Restore en passant status.
    this->en_passant = move->previous_en_passant;
}

// ============================================================================================== 

void Position::move_piece(Move* move)
{
    uint8_t start_square = move->start_location;
    uint8_t end_square = move->end_location;
    uint8_t captured_piece = get_piece(end_square);
    uint64_t start_square_mask = ~(1ULL << (63-start_square));
    uint64_t end_square_mask = 1ULL << (63-end_square);

    // Store captured piece for undoing move.
    move->captured_piece = captured_piece;
    assert(start_square >= 0 && start_square < 64 && end_square >= 0 && end_square < 64);
    
    // The piece we are moving.
    uint8_t moved_piece = move->moving_piece;

    assert(moved_piece < 12);
    
    // Toggle bit in piece board.
    bit_boards[moved_piece] |= end_square_mask;
    bit_boards[moved_piece] &= start_square_mask;

    // Toggle bit in total board.
    bit_boards[TOTAL] |= end_square_mask;
    bit_boards[TOTAL] &= start_square_mask;

    // If necessary, toggle bit in color board.
    if(moved_piece > 5)
    {
        bit_boards[COLOR_BOARD] |= end_square_mask;
        bit_boards[COLOR_BOARD] &= start_square_mask;
    }

    // Check if there was a piece captured.
    if(captured_piece < 12)
    {
        bit_boards[captured_piece] &= ~end_square_mask;

        // Now check if we need to update color board because of capture.
        if(captured_piece > 5)
        {
            bit_boards[COLOR_BOARD] &= ~end_square_mask;
        }
    }
}

// ==============================================================================================

// Process en passant capture seperately.
void Position::handle_en_passant_capture(Move* move)
{
    // Check if move was an en passant capture.
    if (!move->move_takes_an_passant)
        return;
    
    // Start location of moving pawn.
    uint8_t start_location = move->start_location;
    // End location of moving pawn.
    uint8_t end_location = move->end_location;
    // Whether the pawn being moved is black.
    bool taking_pawn_black = move->moving_piece > 5;
    // Board to check. White pawn or black pawn.
    int board_index = W_PAWN + 6*taking_pawn_black;
    // Location of the pawn being captured en passant. One -1 row if black pawn captures or +1 if white pawn captures.
    uint8_t captured_pawn_square = ((end_location + 8) - 16*taking_pawn_black);

    // Check if valid.
    assert(captured_pawn_square >= 0 && captured_pawn_square < 64);
    assert(board_index < 12);
    assert((start_location >= 0 && start_location < 64 && end_location >= 0 && end_location < 64));
    
    // Move pawn.
    toggle_bit_off(bit_boards[board_index], start_location);
    toggle_bit_on(bit_boards[board_index], end_location);
    // Pawn is captured. Toggle bits off for taken piece.
    toggle_bit_off(bit_boards[board_index], captured_pawn_square);
    // Total board taken piece.
    toggle_bit_off(bit_boards[TOTAL], captured_pawn_square);
    toggle_bit_off(bit_boards[TOTAL], start_location);
    // Total board new position.
    toggle_bit_on(bit_boards[TOTAL], end_location);
    // Update color board.
    if(taking_pawn_black)
    {
        toggle_bit_on(bit_boards[COLOR_BOARD], end_location);
        toggle_bit_off(bit_boards[COLOR_BOARD], start_location);
    }
    else
    {
        toggle_bit_off(bit_boards[COLOR_BOARD], end_location + 8);
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
    else if(move->moving_piece == B_PAWN && (move->end_location - move->start_location) == 16 
        || move->moving_piece == W_PAWN && (move->start_location - move->end_location) == 16)
    {
        check_en_passant_possibility(move);
    }
}

// ==============================================================================================

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
            casling_rights &= 0b00000111;
            break;
        case 2: // White queenside
            rook_start = 56;
            rook_end = 59;
            casling_rights &= 0b00001011;
            break;
        case 3: // Black kingside
            rook_start = 7;
            rook_end = 5;
            casling_rights &= 0b00001101;
            break;
        case 4: // Black queenside
            rook_start = 0;
            rook_end = 2;
            casling_rights &= 0b00001110;
            break;
        default:
            return;
    }

    int board_index = W_ROOK + 6*is_black;

    assert(board_index < 12);
    assert((rook_end >= 0 && rook_end < 64 && rook_start >= 0 && rook_start < 64));

    // Move rook and update bitboards.
    toggle_bit_off(bit_boards[board_index], rook_start);
    toggle_bit_on(bit_boards[board_index], rook_end);
    if(is_black)
    {
        toggle_bit_off(bit_boards[COLOR_BOARD], rook_start);
        toggle_bit_off(bit_boards[COLOR_BOARD], rook_end);
    }
    toggle_bit_off(bit_boards[TOTAL], rook_start);
    toggle_bit_off(bit_boards[TOTAL], rook_end);
}

// ==============================================================================================

// Check en passant possibility and update rights.
void Position::check_en_passant_possibility(Move* move)
{
    // Move is a pawn 2 forward, check if an passant is possible.
    // Fetch pieces next to the pawn.
    uint8_t square_on_left = move->end_location-1;
    uint8_t square_on_right = move->end_location+1;
    uint8_t piece_on_left = get_piece(square_on_left);
    uint8_t piece_on_right = get_piece(square_on_right);

    // Check if the moved pawn was black.
    bool moving_piece_black = move->moving_piece > 5;

    // Check if there is an enemy pawn next to the moved pawn.
    bool left_is_pawn = piece_on_left == W_PAWN || piece_on_left == B_PAWN;
    bool right_is_pawn = piece_on_right == W_PAWN || piece_on_right == B_PAWN;
    bool left_is_black = piece_on_left > 5;
    bool right_is_black = piece_on_right > 5;

    // Now check if an passant is possible.
    if(left_is_pawn && left_is_black != moving_piece_black)
    {
        // An passant is possible for pawn on file -1.
        // Left most bit signals that the taking piece is on the left.
        this->en_passant = 0b10000000;
        // Right 6 bits represent the file of the pawn being captured.
        this->en_passant += (move->end_location % 8);
        // Second bit represents the color of the pawn being captured.
        if(moving_piece_black)
            en_passant |= 0b01000000;
    }
    if(right_is_pawn && right_is_black != moving_piece_black)
    {
        // An passant is possible for pawn on +1.
        // An passant is possible for pawn on file -1.
        // Left most bit signals that the taking piece is on the left.
        this->en_passant = 0b00000000;
        // Right 6 bits represent the file of the pawn being captured.
        this->en_passant += (move->end_location % 8);
        // Second bit represents the color of the pawn being captured.
        if(moving_piece_black)
            en_passant |= 0b01000000;
    }
}

// ==============================================================================================

// Generate all possible moves for a color and return them in a vector.
moves Position::determine_moves(bool is_black)
{
    uint64_t enemy_reach = color_reach_board(!is_black);
    possible_moves.move_count = 0;

    // Loop through board to find player's pieces.
    for (uint8_t square = 0; square < 64; square++)
    {
        // get piece type.
        uint8_t piece_type = get_piece(square);

        // Empty square or wrong color.
        if (piece_type >= 12 || is_black != (piece_type > 5))
            continue;

        uint64_t move_squares = make_reach_board(square, is_black, piece_type);

        // Generate moves for the piece.
        generate_piece_moves(square, piece_type, move_squares, is_black, enemy_reach);
    }

    // Check castling rights.
    generate_castling_moves(is_black, enemy_reach);

    // Check en passant.
    generate_en_passant_move(is_black);

    // Return the found moves.
    return possible_moves;
}

// ==============================================================================================

// Generate regular moves.
void Position::generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, uint64_t enemy_reach)
{
    // uint64_t bit_mask = 1ULL << 63;
    for (int i = 0; i < 64; i++)
    {
        // Check if move to square i is possible. Is possible if i intersects with a moving square.
        if(!((1ULL << (63-i))&move_squares))
            continue;
        Move* move = &possible_moves.moves[possible_moves.move_count];
        // Insert move data.
        move->moving_piece = piece_type;
        move->move_takes_an_passant = false;
        move->start_location = pos;
        move->end_location = i;
        
        // TEMP: copy state
        // Position copy(*this);

        // Simulate the move.
        do_move(move);
        // Check if king is not under attack after the move. If not, add move to possible moves.
        possible_moves.move_count += !king_look_around(is_black);
        // Clean up.
        undo_move(move);
    }
}

// ==============================================================================================

// TODO: ignore pawn forward as check.
// Check if king is under check if we don't have an enemy reach board.
// We do this here by simulating different piece moves from the kings position.
// If, from the result, we find that the king can reach that piece type of the enemy player,
// we know it's check.
bool Position::king_look_around(bool is_black)
{   
    uint8_t king_position = find_bit_position(bit_boards[W_KING + 6*is_black]);
    uint64_t king_board = bit_boards[W_KING + 6*is_black];
    bool check = false;

    // Check bishop checks, and diagonal queen checks.
    uint64_t bishop_check = get_bishop_move(king_position, is_black);
    if(boards_intersect(bit_boards[W_BISHOP + !is_black * 6], bishop_check) || boards_intersect(bit_boards[W_QUEEN + !is_black * 6], bishop_check))
        check = true;
    // Check rook checks. And horizontal queen checks.
    uint64_t rook_check = get_rook_move(king_position, is_black);
    if(boards_intersect(bit_boards[W_ROOK + !is_black * 6], rook_check) || boards_intersect(bit_boards[W_QUEEN + !is_black * 6], rook_check))
        check = true;
    // Check knight checks.
    uint64_t knight_check = get_knight_move(king_position, is_black);
    if(boards_intersect(bit_boards[W_KNIGHT + !is_black * 6], knight_check))
        check = true;
    // Check if a pawn is in range of king.
    uint64_t pawn_check = 0b0;
    // Get diagonal attack squares.
    pawn_check |= is_black ? (1ULL << (63-king_position-9) | 1ULL << (63-king_position-7)) & (0xFFULL << (64-(king_position - king_position%8) - 16) & ~(bit_boards[COLOR_BOARD]) & bit_boards[TOTAL]) 
        : (1ULL << (63-king_position+9) | 1ULL << (63-king_position + 7)) & (0xFFULL << (64-(king_position - king_position%8)) & bit_boards[COLOR_BOARD]);
    // make_reach_board(king_position, is_black, W_PAWN + 6*is_black);
    if(boards_intersect(bit_boards[W_PAWN + !is_black * 6], pawn_check))
        check = true;
    // Finally check king intersect.
    uint64_t king_check = get_king_move(king_position, is_black);
    if(boards_intersect(bit_boards[W_KING + !is_black * 6], king_check))
        check = true;

    // if(check)
    //     std::cout << "CHECK" << '\n';

    return check;
}

// ==============================================================================================

// Generate all possible castling moves. 
void Position::generate_castling_moves(bool is_black, uint64_t enemy_reach)
{
    if(king_under_attack(is_black, enemy_reach) || casling_rights == 0)
        return;
    else if (is_black && get_bit(casling_rights, 6))
    {
        uint64_t check_test = bit_boards[B_KING] >> 1;
        uint64_t check_test2 = check_test >> 1;
        // Black kingside castling.
        if (get_piece(5) == EMPTY && get_piece(6) == EMPTY
            && !boards_intersect(check_test, enemy_reach) && !boards_intersect(check_test2, enemy_reach))
        {
            Move move(4, 6);
            move.moving_piece = B_ROOK;
            move.special_cases = 3;
            assert(move.moving_piece < 12);
            if(move.move_bounds_valid())
                possible_moves.moves[possible_moves.move_count++] = move;
        }
    }
    else if (!is_black && get_bit(casling_rights, 4))
    {
        uint64_t check_test = bit_boards[W_KING] >> 1;
        uint64_t check_test2 = check_test >> 1;
        // White kingside castling.
        if (get_piece(61) == EMPTY && get_piece(62) == EMPTY
            && !boards_intersect(check_test, enemy_reach) && !boards_intersect(check_test2, enemy_reach))
        {
            Move move(60, 62);
            move.moving_piece = W_ROOK;
            move.special_cases = 1;
            assert(move.moving_piece < 12);
            if(move.move_bounds_valid())
                possible_moves.moves[possible_moves.move_count++] = move;
        }
    }

    if (is_black && get_bit(casling_rights, 7))
    {
        uint64_t check_test = bit_boards[B_KING] << 1;
        uint64_t check_test2 = check_test << 1;
        // Black queenside castling.
        if (get_piece(1) == EMPTY && get_piece(2) == EMPTY && get_piece(3) == EMPTY
            && !boards_intersect(check_test, enemy_reach) && !boards_intersect(check_test2, enemy_reach))
        {
            Move move(4, 2);
            move.moving_piece = B_ROOK;
            move.special_cases = 4;
            assert(move.moving_piece < 12);
            if(move.move_bounds_valid())
                possible_moves.moves[possible_moves.move_count++] = move;
        }
    }
    else if (!is_black && get_bit(casling_rights, 5))
    {
        uint64_t check_test = bit_boards[W_KING] << 1;
        uint64_t check_test2 = check_test << 1;
        // White queenside castling.
        if (get_piece(59) == EMPTY && get_piece(58) == EMPTY && get_piece(57) == EMPTY
            && !boards_intersect(check_test, enemy_reach) && !boards_intersect(check_test2, enemy_reach))
        {
            Move move(60, 58);
            move.moving_piece = W_ROOK;
            move.special_cases = 2;
            assert(move.moving_piece < 12);
            if(move.move_bounds_valid())
                possible_moves.moves[possible_moves.move_count++] = move;
        }
    }
}

// ==============================================================================================

// Generate en passant moves.
void Position::generate_en_passant_move(bool is_black)
{
    // Check en passant status, 11111111 means no en passant is possible in this position.
    if (en_passant != 0b11111111)
    {
        // last 6 bits are the file of the captured piece.
        uint8_t to = (en_passant & 0b00111111);
        // assert(!(to < 0 || to > 7));
        if(to < 0 || to > 7)
            return;

        // first bit tells us whether the capture is from the left or from the right.
        uint8_t from = to + ((en_passant & 0b10000000) ? -1 : +1);
        // assert(!(from < 0 || from > 7));
        if(from < 0 || from > 7)
            return;

        // Row is on which row the capturing piece is positioned.
        uint8_t start_row = is_black ? 4 : 3;
        uint8_t end_row = is_black ? 5 : 2;

        // Calculate start and end squares for the moving piece.
        uint8_t start_square = from + start_row * 8;
        uint8_t end_square = to + end_row * 8;

        assert((start_square >= 0 && start_square < 64 && end_square >= 0 && end_square < 64));

        // Make the move.
        Move move(start_square, end_square);
        move.moving_piece = W_PAWN + 6*is_black;
        move.move_takes_an_passant = true;
        // Simulate the move.
        assert(move.moving_piece < 12);
        do_move(&move);

        // Check if king is not under attack after the move.
        if (king_look_around(is_black))
        {
            undo_move(&move);
            return;
        }
        assert(move.moving_piece < 12);
        assert(move.moving_piece != INVALID);
        if(move.move_bounds_valid())
            possible_moves.moves[possible_moves.move_count++] = move;
        undo_move(&move);
    }
}

// ==============================================================================================

// Function to check if the bounds of a move are valid.
bool Move::move_bounds_valid()
{
    return  (start_location >= 0 && start_location < 64 && end_location >= 0 && end_location < 64);
}

// ==============================================================================================

// Get the piece on position x, y.
uint8_t Position::get_piece(uint8_t pos) const
{   
    uint64_t bit_mask = 1ULL << (63-pos);
    if(!(pos >= 0 && pos < 64))
        return INVALID;
    else if(!(bit_boards[TOTAL]&bit_mask))
        return EMPTY;
    else if (bit_boards[B_PAWN]&bit_mask)
        return B_PAWN;      // Black pawn
    else if (bit_boards[W_PAWN]&bit_mask)
        return W_PAWN;      // White pawn
    else if (bit_boards[B_KNIGHT]&bit_mask)
        return B_KNIGHT;    // Black knight
    else if (bit_boards[W_KNIGHT]&bit_mask)
        return W_KNIGHT;    // White knight
    else if (bit_boards[B_BISHOP]&bit_mask)
        return B_BISHOP;    // Black bishop
    else if (bit_boards[W_BISHOP]&bit_mask)
        return W_BISHOP;    // White bishop
    else if (bit_boards[B_ROOK]&bit_mask)
        return B_ROOK;      // Black rook
    else if (bit_boards[W_ROOK]&bit_mask)
        return W_ROOK;      // White rook
    else if (bit_boards[B_QUEEN]&bit_mask)
        return B_QUEEN;     // Black queen
    else if (bit_boards[W_QUEEN]&bit_mask)
        return W_QUEEN;     // White queen
    else if (bit_boards[B_KING]&bit_mask)
        return B_KING;      // Black king
    else if (bit_boards[W_KING]&bit_mask)
        return W_KING;      // White king
     
    return INVALID;
}

// ==============================================================================================

// Check if a position is in check after this move is done.
bool Move::is_check(Position* position) const
{
    Move copy_move(*this);
    assert(copy_move.moving_piece < 12);
    assert(copy_move.moving_piece != INVALID);
    assert(copy_move.move_bounds_valid());
    position->do_move(&copy_move);

    bool move_player_black = (moving_piece > 5);
    
    bool is_check = position->king_under_attack( !move_player_black, position->color_reach_board(move_player_black));

    position->undo_move(&copy_move);

    return is_check;
}

// ==============================================================================================

// Check if move is a capture.
bool Move::is_capture(Position* position) const
{
    // We need to check that the color of the start square is different than the end square.
    // Secondly, we need to check that there is a piece on the target square.
    // Otherwise, black piece to empty square would be seen as a capture.

    // Captures if en passant move.
    bool captures = false;

    // Capture if end square color is different than start square color.
    captures = std::max(captures, get_bit_64(position->bit_boards[COLOR_BOARD], start_location) != get_bit_64(position->bit_boards[COLOR_BOARD], end_location));

    // If there is no piece on the end square, move does not capture.
    captures = std::min(captures, get_bit_64(position->bit_boards[TOTAL], end_location));

    // Captures if en passant move.
    captures = std::max(captures, move_takes_an_passant);

    return captures;
}

// ==============================================================================================

// Get capture value of a move.
float Move::capture_value(Position* position) const
{
    return get_piece_value(position->get_piece(this->end_location));
}