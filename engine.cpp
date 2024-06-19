#include "engine.hpp"

float Engine::evaluate_position(Position* position)
{
    float total_eval = 0.f;

    // Calculate difference in pieced value.
    float black_points = evaluate_piece_sum(position, 1);
    float white_points = evaluate_piece_sum(position, 0);

    total_eval += white_points - black_points;

    return total_eval;
}

void Engine::sort_move_priority(std::vector<Move>& moves, Position* position)
{
    std::sort(moves.begin(), moves.end(), [position](const Move& a, const Move& b) 
    {
        // Prioritize captures and checks first.
        bool a_is_capture = a.is_capture(position);
        bool b_is_capture = b.is_capture(position);
        bool a_is_check = a.is_check(position);
        bool b_is_check = b.is_check(position);

        // If one move is a capture and the other is not, prioritize capture.
        if (a_is_capture && !b_is_capture)
            return true;
        if (!a_is_capture && b_is_capture)
            return false;

        // If both are captures or both are not, compare capture values.
        if (a_is_capture && b_is_capture) {
            return a.capture_value(position) > b.capture_value(position); // Higher capture value first.
        }

        // If neither is a capture, prioritize checks.
        if (a_is_check && !b_is_check)
            return true;
        if (!a_is_check && b_is_check)
            return false;

        return false; // Default case: no special priority.
    });
}


EvaluationResult Engine::alpha_beta_pruning(Position* position, bool color_sign, uint8_t depth, float& alpha, float& beta)
{
    std::vector<Move> possible_moves = position->determine_moves(color_sign);
    sort_move_priority(possible_moves, position);
    EvaluationResult result;
    
    // Check if current player loses.
    if(possible_moves.empty())
    {
        result.score = color_sign ? MIN_EVAL : MAX_EVAL;
        return result;
    }
    else if(depth == 0)
    {
        result.score = evaluate_position(position);
        return result;
    }

    // Maximizing player.
    if(color_sign == 0)
    {
        result.score = MIN_EVAL;
        for(Move move : possible_moves)
        {
            // Make copy of the board.
            Position* new_position = new Position(*position);

            // do move.
            new_position->do_move(&move);

            // Recursive call on child.
            EvaluationResult eval = alpha_beta_pruning(new_position, !color_sign, depth-1, alpha, beta);

            // Clean up the new position.
            delete new_position;

            // Evaluate the found score.
            if(eval.score > result.score) 
            {
                result.score = eval.score;
                result.best_move = Move(move);
            }

            alpha = std::max(alpha, eval.score);

            if(beta <= alpha)
                break;
        } 
        return result;
    }
    else
    {
        result.score = MAX_EVAL;
        for(Move move : possible_moves)
        {
            // Make copy of the board.
            Position* new_position = new Position(*position);

            // do move.
            new_position->do_move(&move);

            // Recursive call on child.
            EvaluationResult eval = alpha_beta_pruning(new_position, !color_sign, depth-1, alpha, beta);

            // Clean up the new position.
            delete new_position;

            // Evaluate the found score.
            if(eval.score < result.score) 
            {
                result.score = eval.score;
                result.best_move = Move(move);
            }
            beta = std::min(beta, eval.score);
            
            if(beta <= alpha)
                break;
        }
        return result;
    }
}

float Engine::evaluate_piece_sum(Position* position, uint8_t color_sign)
{
    float points = 0.f;
    for(int i = 0; i < 64; i++)
    {
        uint8_t piece = position->get_piece(i);
        
        if(piece == 0 || get_color(piece) != color_sign)
            continue;

        points += get_piece_value(piece);
    }

    return points;
}