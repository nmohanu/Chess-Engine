#include "util.hpp"
#include <iomanip>
#include <algorithm> 
#include <cassert>

#ifndef POSITION_HPP
#define POSITION_HPP

struct Move;

struct Position
{
    void initialize();

    Position();

    ~Position();

    // Copy constructor.
    Position(const Position& other);

    // Determine possible moves.
    std::vector<Move> determine_moves(bool color_sign);

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos) const;

    // Do a move.
    void do_move(Move* move);

    // Check if king is under attack.
    bool king_under_attack(bool color_sign, uint64_t enemy_reach);
    bool king_look_around(bool is_black);

    // Create attack board for specific piece.
    uint64_t make_reach_board(uint8_t square, bool is_black, uint8_t piece_type);

    // Create attack board for a player.
    uint64_t color_reach_board(bool color_sign);

    // Functions for the pieces' moving logic.
    uint64_t get_pawn_move(uint8_t square, bool is_black);
    uint64_t get_king_move(uint8_t square, bool is_black);
    uint64_t get_bishop_move(uint8_t square, bool is_black);
    uint64_t get_knight_move(uint8_t square, bool is_black);
    uint64_t get_rook_move(uint8_t square, bool is_black);
    uint64_t get_queen_move(uint8_t square, bool is_black);

    // Internal functions.
    void check_en_passant_possibility(Move* move);
    void handle_castling(Move* move);
    void handle_special_cases(Move* move);
    void reset_en_passant_status();
    void handle_en_passant_capture(Move* move);
    void move_piece(Move* move);
    void generate_en_passant_move(bool is_black, std::vector<Move>& possible_moves);
    void generate_castling_moves(bool is_black, std::vector<Move>& possible_moves, uint64_t enemy_reach);
    void generate_piece_moves(int pos, uint8_t piece_type, uint64_t move_squares, bool is_black, std::vector<Move>& possible_moves, uint64_t enemy_reach);

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
    
    // Keep track of player at turn.
    bool white_to_turn = true;

    // By default, castling rights are true. We only use the rightmost 4 bits.
    // From left to right:
    // white kingside, white queenside, black kingside, black queenside.
    uint8_t casling_rights = 0b0000'1111;

    // left most bit indicates whether an passant comes from left or right file. 
    // Second bit is the color sign of the pawn that can be captured.
    // Furthermore, the right most bits indicate the file on which an passant is captured.
    uint8_t en_passant = 0b11111111;


};

struct Board 
{
    Board();

    ~Board();    
    
    Position* position = nullptr;
};

struct Move
{   
    Move();
    Move(Move* other);
    Move(uint8_t start, uint8_t end) : start_location(start), end_location(end) {}

    // Move functions.
    bool is_check(Position* position) const;
    bool is_capture(Position* position) const;
    float capture_value(Position* position) const;
    bool move_bounds_valid();

    // Move data.
    uint8_t start_location;
    uint8_t end_location;
    uint8_t moving_piece = INVALID;
    uint8_t captured_piece = INVALID;

    // Move might be a castling move or engine needs to check for en passant next move.
    // 1 = white kingside, 2 = white queenside, 3 = black kingside, 4 = black queenside, 5 = engine needs to check for an passant afterwards.
    uint8_t special_cases = 0b0;

    bool move_takes_an_passant = false;

    // For move sorting. Lower: more promising.
    int priority_group = 5;

    // Evaluation of position after this move.
    float evaluation;
};

#endif