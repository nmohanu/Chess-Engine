#include "engine.hpp"

Engine::Engine()
{
    hasher.init_zobrist_keys();
}

float Engine::evaluate_position(Position* position)
{
    float total_eval = 0.f;

    // Calculate difference in pieced value.
    float black_points = evaluate_piece_sum(position, 1) * piece_value_weight;
    float white_points = evaluate_piece_sum(position, 0) * piece_value_weight;

    // Evaluate piece positions.
    black_points += evaluate_square_bonus(position, 1) * square_bonus_weight;
    white_points += evaluate_square_bonus(position, 0) * square_bonus_weight;
    
    total_eval = white_points - black_points;
    return total_eval;
}

float Engine::evaluate_square_bonus(Position* position, uint8_t color_sign)
{   
    float total = 0.f;
    for(int i = 0; i < 64; i++)
    {
        uint8_t piece = position->get_piece(i);
        if(color_sign != get_color(piece) || piece == 0)
            continue;
        else
        {
            int it = i;
            if(color_sign)
                it = 63-i;
            switch(piece)
            {
                case B_PAWN:
                case W_PAWN:
                    total += PAWN_BONUS[it];
                    break;
                case B_KNIGHT:
                case W_KNIGHT:
                    total += KNIGHT_BONUS[it];
                    break;
                case B_BISHOP: 
                case W_BISHOP:
                    total += BISHOP_BONUS[it];
                    break;
                case B_ROOK:
                case W_ROOK:
                    total += ROOK_BONUS[it];
                    break;
                case B_QUEEN:
                case W_QUEEN:
                    total += QUEEN_BONUS[it];
                    break;
                case B_KING:
                case W_KING:
                        total += KING_BONUS[it];
                    break;
                default:
                    break;
            }
        }
    }
    return total;
}


void Engine::sort_move_priority(std::vector<Move>& moves, Position* position)
{
    for (Move& move : moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);

        // do move.
        new_position->do_move(&move);

        move.evaluation = evaluate_position(new_position);

        delete new_position;
    }

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) 
    {
        return a.evaluation < b.evaluation;
    });
}

Move Engine::best_move(Position* position, bool color_sign, int depth)
{
    int count = 0;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    float score = 0.f;

    clock_t timer = clock();
    
    Move best_move;

    // if(color_sign)
    // {
    //     score = minimizer(depth, alpha, beta, count, position, best_move, true);
    // }
    // else
    // {
    //     score = maximizer(depth, alpha, beta, count, position, best_move, true);
    // }

    score = search(depth, alpha, beta, count, position, best_move, true, !color_sign);

    timer = clock() - timer;
    float elapsed_seconds = static_cast<float>(timer) / CLOCKS_PER_SEC;
    
    std::cout << "Positions evaluated: " << count << "\n";
    std::cout << "Score found was: " << score << "\n";
    std::cout << "Average positions per second: " << count / elapsed_seconds << '\n';
    std::cout << "Time taken: " << elapsed_seconds << "\n";
    return best_move;
}

float Engine::search(int depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level, bool maximizing)
{
    position_count++;

    if(depth == 0)
        return evaluate_position(position);

    std::vector<Move> possible_moves = position->determine_moves(!maximizing);
    if(possible_moves.empty())
        return maximizing ? INT_MIN : INT_MAX;
    sort_move_priority(possible_moves, position);

    int bound = maximizing ? INT_MIN : INT_MAX;

    Move local_best_move;
    
    for(Move& move : possible_moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);
        new_position->do_move(&move);

        // Make hash copy.
        uint64_t hash = hasher.calculate_zobrist_key(new_position, !maximizing);

        auto transposition_entry = transposition_table.get(hash);
        if (transposition_entry) 
        {
            float eval = transposition_entry->score;
            if(process_alpha_beta(alpha, beta, maximizing, eval, bound))
                local_best_move = Move(move);
            if (alpha >= beta)
                break;
            continue;
        }

        float eval = search(depth-1, alpha, beta, position_count, new_position, best_move, false, !maximizing);

        // transposition_table.insert(hash, depth, eval);

        delete new_position;

        if(process_alpha_beta(alpha, beta, maximizing, eval, bound))
            local_best_move = Move(move);

        if (alpha >= beta)
            break;
        continue;
    }

    if(top_level)
        best_move = local_best_move;

    return bound;
}

bool Engine::process_alpha_beta(int& alpha, int& beta, bool maximizing, int eval, int& bound)
{
    if (maximizing && eval > bound) 
    {
        bound = eval;
        return true;
    }
    if(maximizing && eval >= alpha)  
        alpha = eval;
    else if (!maximizing && eval < bound)
    {
        bound = eval;
        return true;
    }
    if(!maximizing && eval <= beta) 
        beta = eval;
    return false;
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

// void Engine::zobrist_hash(Position* position, uint64_t old_hash, Move* move)
// {
//     // uint64_t hash = 0b0;

// }