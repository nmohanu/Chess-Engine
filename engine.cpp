#include "engine.hpp"

Engine::Engine()
{
    hasher.init_zobrist_keys();
}

void Engine::do_perft_test(int depth)
{
    Position position;
    position.initialize();
    clock_t start = clock();
    uint64_t nodes = perft_test(&position, depth, false);
    clock_t end = clock();
    double time_cost = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "PERFT results: \nNodes evaluated: " << nodes << "\nTime cost: " << time_cost << '\n';
    std::cout << "Nodes per second " << nodes / time_cost << '\n';
}

uint64_t Engine::perft_test(Position* position, int depth, bool color_sign)
{
    // Determine possible moves.
    std::vector<Move> possible_moves = position->determine_moves(color_sign);
    uint64_t nodes = 0;

    // Base case.
    if(depth == 1)
        return possible_moves.size();

    for(Move& move : possible_moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);
        // Do move.
        new_position->do_move(&move);
        // Recursive call.
        nodes += perft_test(new_position, depth-1, !color_sign);
        // Undo move.
        delete new_position;
    }
    // Return result.
    return nodes;
}

// Function to return the best found move.
void Engine::best_move(Position* position, bool color_sign, int depth, Move& best_move)
{
    // For analysis.
    int count = 0;
    int zobrist_skips = 0;
    clock_t timer = clock();

    // Initialize
    int alpha = -100;
    int beta = 100;
    float score = 0.f;
    
    Move best_found;

    // Find best move.
    score = search(depth, alpha, beta, count, position, best_found, true, !color_sign, zobrist_skips, 0);

    // Chech if engine was stopped due to time.
    if(score == -999999)
    {
        return;
    }

    previous_score = score;

    // Analysis.
    timer = clock() - timer;
    float elapsed_seconds = static_cast<float>(timer) / CLOCKS_PER_SEC;
    
    // Output results.
    std::cout << "Positions evaluated: " << count << "\n";
    std::cout << "Score found was: " << score << "\n";
    std::cout << "Average positions per second: " << count / elapsed_seconds << '\n';
    std::cout << "Time taken: " << elapsed_seconds << "\n";
    std::cout << "Nodes skipped thanks to mr. Zobrist: " << zobrist_skips << "\n";

    // Check if a valid move was found. 64 is the signal that none was found.
    assert(best_move.start_location != 64);
    best_move = Move(best_found);
    return;
}

float Engine::search(int current_depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level, bool maximizing, int& zobrist_skips, int depth_limit)
{
    // Initialize ==================================================================================================================

    // Check if the time is up. If so, signal this to parent calls.
    if(time_up)
        return -999999;

    // Make hash entries of position.
    int hashf = hashfALPHA;
    uint64_t key = hasher.calculate_zobrist_key(position, !maximizing);
    int entry_key_value = transposition_table.read_hash_entry(alpha, beta, current_depth, key);

    // Read hash entry.
    if(entry_key_value != no_hash_entry && !top_level)
    {
        // Position wes already evaluated in a different order.
        zobrist_skips++;
        return entry_key_value;
    }

    // Count unique positions visited.
    position_count++;

    // End of depth, evaluate position.
    if(current_depth <= depth_limit)
    {
        int val = evaluate_position(position);
        transposition_table.insert_hash(current_depth, val, hashfEXACT, key);
        return val;
    }

    // Determine possible moves.
    std::vector<Move> possible_moves = position->determine_moves(!maximizing);

    // No moves available means current player loses.
    if(possible_moves.empty())
        return maximizing ? -100 : 100;

    sort_move_priority(possible_moves, position);

    float eval = maximizing ? -100 : 100;

    Move local_best_move = possible_moves.front();

    // bool first_node = true;

    // Actual search. ==================================================================================================================
    
    for(Move& move : possible_moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);
        // Do move.
        new_position->do_move(&move);
        // Evaluate.
        float score;
        score = search(current_depth - 1, alpha, beta, position_count, new_position, best_move, false, !maximizing, zobrist_skips, depth_limit);
        // Undo.
        delete new_position;
        // Check if time is up.
        if(score == -999999)
            return -999999;

        // Evaluate found score and edit alpha, beta, and best move accordingly.
        if (maximizing)
        {
            if (score >= eval)
            {
                
                eval = score;
                if (eval > alpha)
                {
                    alpha = eval;
                    local_best_move = Move(move);
                    hashf = hashfEXACT;
                }
            }
            if (eval >= beta)
            {
                transposition_table.insert_hash(current_depth, eval, hashfBETA, key);
                break;
            }
        }
        else
        {
            if (score <= eval)
            {
                
                eval = score;
                if (eval < beta)
                {
                    beta = eval;
                    local_best_move = Move(move);
                    hashf = hashfEXACT;
                }
            }
            if (eval <= alpha)
            {
                transposition_table.insert_hash(current_depth, eval, hashfALPHA, key);
                break;
            }
        }
    }

    if(top_level)
        best_move = local_best_move;

    transposition_table.insert_hash(current_depth, eval, hashf, key);
    return eval;
}

void Engine::sort_move_priority(std::vector<Move>& moves, Position* position)
{
    for (Move& move : moves)
    {
        // Make copy of the board.
        Position* new_position = new Position(*position);

        // Apply the move to the copied position.
        new_position->do_move(&move);

        // Evaluate the position after the move.
        
        
        // Check if the move gives check.
        bool gives_check = move.is_check(new_position);
        
        // Set priority groups based on properties.
        if (move.is_capture(position)) 
        {
            move.priority_group = 1;
        } else if (gives_check) 
        {
            move.priority_group = 2;
        } else 
        {
            move.priority_group = 3;
        }

        // Capture value for sorting capture moves.
        if (move.priority_group == 2) 
        {
            move.capture_val = move.capture_value(position);
        }

        delete new_position;
    }

    // Sort vector according to priority groups.
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        // Sort by priority_group first.
        if (a.priority_group != b.priority_group) {
            return a.priority_group < b.priority_group;
        }

        return a.evaluation > b.evaluation; 
    });
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
