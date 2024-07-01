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
    // Initialize rook and bishop masks.
    // for(int square = 0; square < 64; square++)
    // {
    //     // Make masks for all squares. 
    //     // bishop_masks[square] = make_bishop_mask(square);
    //     // rook_masks[square] = make_rook_mask(square);
    // }

    // Make lookup tables.
    // for (int square = 0; square < 64; square++) 
    // {
    //     uint64_t bishop_mask = bishop_masks[square];
    //     uint64_t bishop_relevant_bits = __builtin_popcountll(bishop_mask);
    //     std::vector<uint64_t> bishop_blocker_boards = create_all_blocker_boards(bishop_mask);

        
    //     //Fill lookup table for bishop.
    //     for(int board = 0; board < bishop_blocker_boards.size(); board++)
    //     {
    //         uint64_t blocker_board = bishop_blocker_boards[board];
    //         int index = (blocker_board * bishop_magic_numbers[63-square]) >> (64 - bishop_relevant_bits);
    //         bishop_attacks[square][index] = bishop_attack_on_fly(square, blocker_board);
    //         // print_bitboard(bishop_attacks[square][index]);
    //     }
    // }
}

Position::Position(const Position& other) 
{
    // Copy contents of position.
    this->casling_rights = other.casling_rights;
    this->en_passant = other.en_passant;
    this->white_to_turn = other.white_to_turn;

    // Copy bitboards.
    for(uint8_t piece = W_KING; piece < 14; piece++) this->bit_boards[piece] = other.bit_boards[piece];
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

    attack_board |= generators[piece_type - 6*is_black](square, is_black, bit_boards[TOTAL], bit_boards[COLOR_BOARD]);

    return attack_board;
}

// ==============================================================================================

// Create the attack board for all pieces of a player.
uint64_t Position::color_reach_board(bool is_black)
{
    uint64_t attack_board = 0ULL;
    // uint64_t bit_mask = 1ULL << 63;

    uint64_t total_board = bit_boards[TOTAL];

    if(is_black)
        total_board &= bit_boards[COLOR_BOARD];
    else
        total_board &= ~bit_boards[COLOR_BOARD];

    // uint64_t mask = -(int64_t)is_black;
    // total_board &= (bit_boards[COLOR_BOARD] & mask) | (~bit_boards[COLOR_BOARD] & ~mask);

    while(__builtin_popcountll(total_board) >= 1)
    {
        uint8_t pos = __builtin_clzll(total_board);
        // Add piece reach to total reach.
        attack_board |= make_reach_board(pos, is_black, get_piece(pos));

        total_board &= ~(1ULL << (63 - pos));
    }

    // for(int i = 0; i < 64; i++)
    // {
    //     uint8_t pos = 63-i;
    //     uint64_t mask = 1ULL << (i);
        
    //     if(boards_intersect(mask, total_board))
    //     {
    //         attack_board |= make_reach_board(pos, is_black, get_piece(pos));
    //     }
    // }

    // while(__builtin_popcountll(total_board) >= 1)
    // {
    //     uint8_t pos = __builtin_clzll(total_board);
    //     // Add piece reach to total reach.
    //     attack_board |= make_reach_board(pos, is_black, get_piece(pos));

    //     total_board &= total_board-1;
    // }
    
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
    this->casling_rights = move->previous_castling_rights;
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

    // Update castling rights if king was moved.
    if(moved_piece == W_KING || moved_piece == B_KING)
    {
        uint8_t mask = 0b11;

        if(start_square == 4)
            casling_rights &= ~(mask);
        else if(start_square == 60)
            casling_rights &= ~(mask << 2);
    }
    // Update castling rights if a rook was moved or captured.
    else if(moved_piece == W_ROOK || moved_piece == B_ROOK || captured_piece == W_ROOK || captured_piece == B_ROOK)
    {
        uint8_t mask = 1ULL;
        if(start_square == 0)
            casling_rights &= ~(mask);
        else if(start_square == 7)
            casling_rights &= ~(mask << 1);
        else if(start_square == 56)
            casling_rights &= ~(mask << 2);
        else if(start_square == 63)
            casling_rights &= ~(mask << 3);
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
            rook_end = 3;
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
        toggle_bit_on(bit_boards[COLOR_BOARD], rook_end);
    }
    toggle_bit_off(bit_boards[TOTAL], rook_start);
    toggle_bit_on(bit_boards[TOTAL], rook_end);
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
    if(left_is_pawn && left_is_black != moving_piece_black && move->end_location%8 != 0)
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
    if(right_is_pawn && right_is_black != moving_piece_black && move->end_location%8 != 7)
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
void Position::determine_moves(bool is_black, moves& possible_moves)
{
    
    uint64_t mask = -(int64_t)is_black;

    uint64_t enemy_reach = color_reach_board(!is_black);

    for(int i = 0 + 6*is_black; i < 12 - 6*!is_black; i++)
    {
        uint8_t piece_type = i;
        uint64_t board = bit_boards[i];
        
        while(__builtin_popcountll(board) >= 1)
        {
            uint8_t square = __builtin_clzll(board);
        
            uint64_t move_squares = generators[piece_type - 6*is_black](square, is_black, bit_boards[TOTAL], bit_boards[COLOR_BOARD]);
            uint64_t own_pieces = is_black ? bit_boards[COLOR_BOARD] : (~bit_boards[COLOR_BOARD] & bit_boards[TOTAL]);
            move_squares &= ~own_pieces;

            // Generate moves for the piece.
            generate_piece_moves(square, piece_type, move_squares, is_black, enemy_reach, possible_moves);

            board &= ~(1ULL << (63 - square));
        }
    }

    // Check castling rights.
    generate_castling_moves(is_black, enemy_reach, possible_moves);

    // Check en passant.
    generate_en_passant_move(is_black, possible_moves);
}

// ==============================================================================================

// Generate regular moves.
void Position::generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, uint64_t enemy_reach, moves& possible_moves)
{

    while(__builtin_popcountll(move_squares) >= 1)
    {
        uint8_t i = __builtin_clzll(move_squares);
        // uint8_t end_loc = 63-i;

        Move* move = &possible_moves.moves[possible_moves.move_count];
        // Insert move data.
        move->moving_piece = piece_type;
        move->move_takes_an_passant = false;
        move->start_location = pos;
        move->end_location = i;
        move->special_cases = 0b0;
        move->previous_castling_rights = casling_rights;

        // Check if king is not under attack after the move. If not, add move to possible moves.
        possible_moves.move_count += move_legal(move, move_squares, is_black, enemy_reach);
        move_squares &= ~(1ULL << (63 - i));
    }
}

// ==============================================================================================

// Check if a move is legal.
bool Position::move_legal(Move* move, uint64_t move_squares, bool is_black, uint64_t enemy_reach)
{
    uint64_t start_board = 1ULL << (63-move->start_location);
    uint64_t end_board = 1ULL << (63-move->end_location);
    
    
    if(move->moving_piece == (W_KING + 6*is_black))
    {   // Simulate move.
        do_move(move);
        bool check = king_look_around(is_black, find_bit_position(bit_boards[W_KING + 6*is_black]));
        undo_move(move);
        return !check;
    }
    // If moving piece is not  the king, we need to check if the moving piece is not pinned.
    // First, if moving piece and king both are not attacks, move is legal.
    else if(!boards_intersect(start_board, enemy_reach) && !boards_intersect(bit_boards[W_KING + 6*is_black], enemy_reach))
    {
        return true;
    }
    // Piece is attaacked, now we need to check if it's pinned. Simulate move.
    else
    {
        do_move(move);
        bool check = king_look_around(is_black, find_bit_position(bit_boards[W_KING + 6*is_black]));
        undo_move(move);
        return !check;
    }
}

// Check if king is under check if we don't have an enemy reach board.
// We do this here by simulating different piece moves from the kings position.
// If, from the result, we find that the king can reach that piece type of the enemy player,
// we know it's check.
bool Position::king_look_around(bool is_black, uint8_t square)
{   
    uint64_t pawn_squares = get_pawn_attack(is_black, square, bit_boards[COLOR_BOARD], bit_boards[TOTAL]);

    return          (pawn_squares                                                       &       bit_boards[W_PAWN + !is_black * 6])
            ||      (get_knight_move(square, is_black, bit_boards[TOTAL], 0)            &       bit_boards[W_KNIGHT + !is_black * 6])
            ||      (get_rook_move(square, is_black, bit_boards[TOTAL], 0)              &       (bit_boards[W_ROOK + !is_black * 6]     |   bit_boards[W_QUEEN + !is_black * 6]))
            ||      (get_bishop_move(square, is_black, bit_boards[TOTAL], 0)            &       (bit_boards[W_BISHOP + !is_black * 6]   |   bit_boards[W_QUEEN + !is_black * 6]))
            ||      (get_king_move(square, is_black, bit_boards[TOTAL], 0)              &       bit_boards[W_KING + !is_black * 6]);
}

// ==============================================================================================

// Generate all possible castling moves. 
void Position::generate_castling_moves(bool is_black, uint64_t enemy_reach, moves& possible_moves)
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
            move.moving_piece = B_KING;
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
            move.moving_piece = W_KING;
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
            move.moving_piece = B_KING;
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
            move.moving_piece = W_KING;
            move.special_cases = 2;
            assert(move.moving_piece < 12);
            if(move.move_bounds_valid())
                possible_moves.moves[possible_moves.move_count++] = move;
        }
    }
}

// ==============================================================================================

// Generate en passant moves.
void Position::generate_en_passant_move(bool is_black, moves& possible_moves)
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
        move.previous_castling_rights = casling_rights;
        move.previous_en_passant = en_passant;
        // Simulate the move.
        assert(move.moving_piece < 12);
        do_move(&move);

        // Check if king is not under attack after the move.
        if (king_look_around(is_black, find_bit_position(bit_boards[W_KING + 6*is_black])))
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

    if(!(bit_boards[TOTAL]&bit_mask))
        return EMPTY;

    uint8_t piece = 0;

    for(int i = 0; i < 12; i++) piece += !(bit_boards[i] & bit_mask) && (i == piece);
    
    return piece;
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

std::string Move::to_string()
{
    std::string start_notation = make_chess_notation(this->start_location);
    std::string destination_notation = make_chess_notation(this->end_location);
    return start_notation + destination_notation;
}

