#include "transposition_table.hpp"
#include <math.h>
#include <stack>
#include <array>
#include <ctime>
#include <algorithm>
#include <iostream>

#ifndef ENGINE_HPP
#define ENGINE_HPP

class Engine 
{
public:

    // Call recursive functions to determine best move.
    void best_move(Position* position, bool color_sign, int depth,  Move& best_move);

    void do_perft_test(int depth, Position* position, bool white_to_move);

    uint64_t perft_test(Position* position, int depth, bool color_sign, moves& possible_moves);

    bool time_up = false;

    Engine();

private:
    // Working but can be improved later:

    float search(int current_depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level, bool maximizing, int depth_limit, moves possible_moves);

    float evaluate_piece_sum(Position* position, uint8_t color_sign);

    float evaluate_position(Position* position);

    // TODO:

    float evaluate_square_bonus(Position* position, uint8_t color_sign);

    float evaluate_piece_value(Position* position, uint8_t square);

    float evaluate_color(Position* position, uint8_t color_sign);

    float evaluate_mobility(Position* position, uint8_t color_sign);

    float evaluate_outpost_bonus(Position* position, uint8_t color_sign);

    float evaluate_king_protection_bonus(Position* position, uint8_t color_sign);

    float evaluate_king_position(Position* position, uint8_t color_sign);
    
    float evaluate_king_threat(Position* position, uint8_t color_sign);

    float evaluate_protection_bonus(Position* position, uint8_t color_sign);

    float evaluate_rooks_positions(Position* position, uint8_t color_sign);

    float evaluate_bisshops_positions(Position* position, uint8_t color_sign);

    float evaluate_knights_positions(Position* position, uint8_t color_sign);

    float evaluate_queen_position(Position* position, uint8_t color_sign);

    float evaluate_pawns_positions(Position* position, uint8_t color_sign);

    void sort_move_priority(std::vector<Move>& moves, Position* position);

    // Params.

    const float bishhop_pair_weight= 1.f;

    const float doubled_pawn_weight = 1.f;
    const float isolated_pawn_weight = 1.f;
    const float passed_pawn_weight = 1.f;
    const float backwards_pawn_weight = 1.f;
    const float good_rook_weight = 1.f;
    const float pin_weight = 1.f;
    const float skewer_weight = 1.f;
    const float possible_checks_weight = 1.f;
    const float piece_value_weight = 2.f;
    const float square_bonus_weight = 0.5f;

    TranspositionTable transposition_table;

    ZobristHash hasher;

    int currently_evaluating_perft_depth;
};

#endif