#include "engine.hpp"

float Engine::evaluate_position(Position* position)
{
    return 0.f;
}

float Engine::evaluate_piece_ratio(Position* position, uint8_t color_sign)
{
    float black;
    float white;
    for(int i = 0; i < 64; i++)
    {
        uint8_t piece = position->get_piece(i);
        switch (piece)
        {
        case W_BISHOP:
            white += BISHOP_VALUE;
            break;
        case B_BISHOP:
            black += BISHOP_VALUE;
            break;
        case W_KNIGHT:
            white += KNIGHT_VALUE;
            break;
        case B_KNIGHT:
            black += KNIGHT_VALUE;
            break;
        case W_ROOK:
            white += ROOK_VALUE;
            break;
        case B_ROOK:
            black += ROOK_VALUE;
            break;
        case W_QUEEN:
            white += QUEEN_VALUE;
        case B_QUEEN:
            black += QUEEN_VALUE;
        case W_PAWN:
            white += PAWN_VALUE;
        case B_PAWN:
            black += PAWN_VALUE;
        
        default:
            break;
        }
    }

    return std::roundf(white/black * 100) / 100;
}