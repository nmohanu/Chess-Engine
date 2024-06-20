#include "util.hpp"
#include <iomanip>

#ifndef POSITION_HPP
#define POSITION_HPP

struct Move;

struct Position
{
public:

    void initialize();

    Position();

    ~Position();

    Position(const Position& other);

    // Determine possible moves.
    std::vector<Move> determine_moves(bool color_sign);
    
    // Replace 4 bits by 4 new bits.
    void set_piece(uint8_t new_piece, uint8_t pos);

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos) const;

    // Do a move.
    void do_move(Move* move);

    // Check if king is under attack.
    bool king_under_attack(bool color_sign);

    // Create attack board for specific piece.
    uint64_t make_reach_board(uint8_t x, uint8_t y);

    // Create moving board for specific piece.
    uint64_t make_move_board(uint8_t, uint8_t);

    // Create attack board for a player.
    uint64_t color_reach_board(bool color_sign);

    // Get position of a piece.
    uint8_t get_piece_position(uint8_t piece);

    // Functions to get the attack / defend reach of a piece.
    uint64_t get_pawn_reach(uint8_t x, uint8_t y, bool is_white);
    uint64_t get_king_reach(uint8_t x, uint8_t y);
    uint64_t get_bishop_reach(uint8_t x, uint8_t y);
    uint64_t get_knight_reach(uint8_t x, uint8_t y);
    uint64_t get_rook_reach(uint8_t x, uint8_t y);
    uint64_t get_queen_reach(uint8_t x, uint8_t y);

    // Functions for the pieces' moving logic.
    uint64_t get_pawn_move(uint8_t x, uint8_t y, bool is_white);
    uint64_t get_king_move(uint8_t x, uint8_t y);
    uint64_t get_bishop_move(uint8_t x, uint8_t y);
    uint64_t get_knight_move(uint8_t x, uint8_t y);
    uint64_t get_rook_move(uint8_t x, uint8_t y);
    uint64_t get_queen_move(uint8_t x, uint8_t y);

    // false = black, true = white.
    uint64_t first_16;
    uint64_t second_16;
    uint64_t third_16;
    uint64_t fourth_16;

    // By default, castling rights are true. We only use the rightmost 4 bits.
    // From left to right:
    // white kingside, white queenside, black kingside, black queenside.
    uint8_t casling_rights = 0b0000'1111;

    // left most bit indicates whether an passant comes from left or right file. 
    // Second bit is the color sign of the pawn that can be captured.
    // Furthermore, the right most bits indicate the file on which an passant is captured.
    uint8_t en_passant = 0b11111111;

    Move* last_move = nullptr;
};

class Board 
{
public:
    Board();

    ~Board();    
    
    Position* position = nullptr;


};

struct Move
{   
    Move();
    Move(const Move& other);
    Move(uint8_t start, uint8_t end) : start_location(start), end_location(end) {}

    bool is_check(Position* position) const;
    bool is_capture(Position* position) const;
    float capture_value(Position* position) const;

    uint8_t start_location;
    uint8_t end_location;

    // Move might be a castling move.
    // 1 = white kingside, 2 = white queenside, 3 = black kingside, 4 = black queenside, 5 = engine needs to check for an passant afterwards.
    uint8_t special_cases = 0b0;

    uint8_t move_takes_an_passant;
};

#endif