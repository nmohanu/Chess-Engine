#include "board.hpp"
#include "thread"

class Engine 
{
public:
    float evaluate_piece_value(Position* position, uint8_t square);

    float evaluate_position(Position* position);

    float evaluate_position_Rec(Position* position);

    float evaluate_color(Position* position, uint8_t color_sign);

private:
    // Params.
    const int ROOK_VALUE = 5;
    const int QUEEN_VALUE = 9;
    const int KNIGHT_VALUE = 3;
    const int BISHOP_VALUE = 3;
    const int PAWN_VALUE = 1;

    std::thread worker1;
    std::thread worker2;
};