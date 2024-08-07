#include "move.hpp"

#ifndef POSITION_HPP
#define POSITION_HPP

// ==============================================================================================

// Function pointer arrays.
typedef void (Position::*move_function) (int, uint8_t, uint64_t, bool, uint64_t, moves&);

// ==============================================================================================

// Position.
struct Position
{
    // ==============================================================================================

    // Constructor destructor
    Position();
    ~Position();
    // Copy constructor.
    Position(const Position& other);

    // ==============================================================================================

    // Move generation functions.
    // Determine possible moves.
    void determine_moves(bool color_sign, moves& moves);
    // Generate moves for a piece.
    void generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, uint64_t enemy_reach, moves& moves);
    // Generate moves for a pawn.
    void generate_pawn_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, uint64_t enemy_reach, moves& moves);
    // Special cases.
    void generate_en_passant_move(bool is_black, moves& moves);
    void generate_castling_moves(bool is_black, uint64_t enemy_reach, moves& moves);

    // ==============================================================================================

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos) const;

    // ==============================================================================================

    // Do a move.
    void do_move(Move* move);
    void check_en_passant_possibility(Move* move);
    void handle_castling(Move* move);
    void handle_special_cases(Move* move);
    void reset_en_passant_status();
    void handle_en_passant_capture(Move* move);
    void move_piece(Move* move);

    // ==============================================================================================

    // Undo a move.
    void undo_move(Move* move);
    void undo_piece_move(Move* move);
    void undo_en_passant_capture(Move* move);
    void restore_special_cases(Move* move);
    void restore_en_passant_and_castling(Move* move);

    // ==============================================================================================

    // Check if king is under attack.
    bool king_under_attack(bool color_sign, uint64_t enemy_reach);
    bool king_look_around(bool is_black, uint8_t square);
    bool move_legal(Move* move, uint64_t move_squares, bool is_black, uint64_t enemy_reach);

    // ==============================================================================================

    // Create attack board for specific piece.
    uint64_t make_reach_board(uint8_t square, bool is_black, uint8_t piece_type);
    // Create attack board for a player.
    uint64_t color_reach_board(bool color_sign);

    // ==============================================================================================

    // Function arrays.
    generator_function generators[6] = 
    {
        get_king_move,
        get_queen_move,
        get_rook_move,
        get_bishop_move,
        get_knight_move,
        get_pawn_move
    };

    move_function move_functions[2] = 
    {
        generate_piece_moves,
        generate_pawn_moves
    };

    // ==============================================================================================

    void print_to_terminal();

    // ==============================================================================================

    // Represent the board as bits.
    // Index is equal to the piece number defenition. 
    uint64_t bit_boards[14] = 
    {
        KING_SQUARES & ~BLACK_PIECES,    // W_KING
        QUEEN_SQUARES & ~BLACK_PIECES,   // W_QUEEN
        ROOK_SQUARES & ~BLACK_PIECES,    // W_ROOK
        BISHOP_SQUARES & ~BLACK_PIECES,  // W_BISHOP
        KNIGHT_SQUARES & ~BLACK_PIECES,  // W_KNIGHT
        PAWN_SQUARES & ~BLACK_PIECES,    // W_PAWN
        KING_SQUARES & BLACK_PIECES,     // B_KING
        QUEEN_SQUARES & BLACK_PIECES,    // B_QUEEN
        ROOK_SQUARES & BLACK_PIECES,     // B_ROOK
        BISHOP_SQUARES & BLACK_PIECES,   // B_BISHOP
        KNIGHT_SQUARES & BLACK_PIECES,   // B_KNIGHT
        PAWN_SQUARES & BLACK_PIECES,     // B_PAWN
        TOTAL_SQUARES,                   // All pieces
        BLACK_PIECES                     // Black pieces.
    };
    
    // ==============================================================================================

    // Position data.
    // Keep track of player at turn.
    bool white_to_turn = true;

    // By default, castling rights are true. We only use the rightmost 4 bits.
    // From left to right:
    // white kingside, white queenside, black kingside, black queenside.
    uint8_t casling_rights = 0b0000'1111;

    // left most bit indicates whether an passant comes from left or right file. 
    // Second bit is the color sign of the pawn that can be captured.
    // Furthermore, the right most bits indicate the file on which an passant is captured.
    uint8_t en_passant = 0b00000000;
    
};

// ==============================================================================================

struct Board 
{
    Board();

    ~Board();    
    
    Position* position = nullptr;
};

// ==============================================================================================

#endif