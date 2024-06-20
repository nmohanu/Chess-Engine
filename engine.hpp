#include "board.hpp"
#include "thread"
#include <math.h>
#include <stack>
#include <array>

struct TranspositionTable
{
    uint64_t current_hash = 0b0;

};

struct ZobristHash
{
    void init_zobrist_keys();

    uint64_t calculate_zobrist_key(Position* position);

    enum PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
    enum Color { WHITE, BLACK };

    std::array<std::array<uint64_t, 12>, 64> zobrist_pieces;

    std::array<uint64_t, 8> zobrist_en_passant_file;

    uint64_t zobrist_black_to_move;
};

class Engine 
{
public:

    // Call recursive functions to determine best move.
    Move best_move(Position* position, bool color_sign, int depth);

private:

    // Working but can be improved later:

    float maximizer(int depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level);

    float minimizer(int depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level);

    float evaluate_piece_sum(Position* position, uint8_t color_sign);

    // TODO:

    void zobrist_hash(Position* position);

    float evaluate_piece_value(Position* position, uint8_t square);

    float evaluate_position(Position* position);

    float evaluate_color(Position* position, uint8_t color_sign);

    float evaluate_mobility(Position* position, uint8_t color_sign);

    float evaluate_square_bonus(Position* position, uint8_t color_sign);

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

    TranspositionTable transposition_table;

};

