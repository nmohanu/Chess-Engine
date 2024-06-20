#include "engine.hpp"

Engine::Engine()
{
    hasher.init_zobrist_keys();

    // Initiate a hash with the starting position.
    Position* starting_position = new Position();
    starting_position->initialize();
    transposition_table.current_hash = hasher.calculate_zobrist_key(starting_position, 0);
    delete starting_position;
}

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

Move Engine::best_move(Position* position, bool color_sign, int depth)
{
    int count = 0;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    float score = 0;
    Move best_move;

    if(color_sign)
    {
        score = minimizer(depth, alpha, beta, count, position, best_move, true);
    }
    else
    {
        score = maximizer(depth, alpha, beta, count, position, best_move, true);
    }
    
    std::cout << "Positions evaluated: " << count << "\n";
    std::cout << "Score found was: " << score << "\n";
    return best_move;
}

float Engine::maximizer(int depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level)
{
    position_count++;

    if(depth == 0)
        return evaluate_position(position);

    std::vector<Move> possible_moves = position->determine_moves(0);
    // Check if current player loses.
    if(possible_moves.empty())
        return INT_MIN;

    sort_move_priority(possible_moves, position);
    
    int max_eval = INT_MIN;
    Move local_best_move;

    for(Move& move : possible_moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);

        // Make hash copy.
        uint64_t hash_copy = transposition_table.current_hash;

        // do move.
        new_position->do_move(&move);

        // Update hash.
        hasher.update_zobrist_hash(&move, new_position, 0, hash_copy);

        // If hash is in hash table, skip this move. 
        if(transposition_table.contains(hash_copy))
            continue;
        else
            transposition_table.insert(hash_copy);

        // Recursive call on child.
        int eval = minimizer(depth-1, alpha, beta, position_count, new_position, best_move, false);

        // Restore hash key.
        transposition_table.current_hash = hash_copy;

        // Clean up the new position.
        if(new_position != nullptr)
            delete new_position;
        new_position = nullptr;

        if(eval > max_eval) 
        {
            max_eval = eval;
            local_best_move = Move(move); 
        }

        // Evaluate the found score.
        if(eval >= alpha)  
            alpha = eval;

        if(alpha >= beta)
        {
            break;
        }
    } 

    if(top_level)
    {
        best_move = local_best_move;
    }

    return max_eval;
}

float Engine::minimizer(int depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level)
{
    position_count++;

    if(depth == 0)
        return evaluate_position(position);

    std::vector<Move> possible_moves = position->determine_moves(1);
    // Check if current player loses.
    if(possible_moves.empty())
        return INT_MAX;

    sort_move_priority(possible_moves, position);

    int min_eval = INT_MAX;
    Move local_best_move;

    for(Move& move : possible_moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);

        // Make hash copy.
        uint64_t hash_copy = transposition_table.current_hash;

        // do move.
        new_position->do_move(&move);

        // Update hash.
        hasher.update_zobrist_hash(&move, new_position, 0, hash_copy);

        // If hash is in hash table, skip this move. 
        if(transposition_table.contains(hash_copy))
        {
            delete new_position;
            continue;
        }
        else
            transposition_table.insert(hash_copy);

        // Recursive call on child.
        int eval = maximizer(depth-1, alpha, beta, position_count, new_position, best_move, false);

        // Restore hash key.
        transposition_table.current_hash = hash_copy;

        if(eval < min_eval)
        {
            min_eval = eval;
            local_best_move = Move(move);
        }

        // Clean up the new position.
        if(new_position != nullptr)
            delete new_position;
        new_position = nullptr;

        // Evaluate the found score.
        if(eval <= beta) 
            beta = eval;

        if(beta <= alpha)
        {
            break;
        }
    } 

    if(top_level)
    {
        best_move = local_best_move;
    }

    return min_eval;
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