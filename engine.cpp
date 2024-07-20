#include "engine.hpp"

Engine::Engine()
{
    hasher.init_zobrist_keys();
}

void Engine::do_perft_test(int depth, Position* position, bool white_to_move)
{
    transposition_table.clear_table();
    clock_t start = clock();
    int captures = 0;
    int checks = 0;
    int en_passants = 0;
    currently_evaluating_perft_depth = depth;
    moves possible_moves;
    possible_moves.move_count = 0;

    uint64_t nodes = perft_test(position, depth-1, !white_to_move, captures, checks, en_passants, possible_moves);
    clock_t end = clock();
    double time_cost = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Depth: " << depth << '\n';
    std::cout << "PERFT results: \nNodes evaluated: " << nodes <<
        "\nTime cost: " << time_cost << '\n';
    std::cout << "Nodes per second " << (nodes / 1000000) / time_cost << " Million nodes per second" << '\n';
    std::cout << "================================================================================ \n";
}

uint64_t Engine::perft_test(Position* position, int depth, bool color_sign, int& captures, int& checks, int& en_passants, moves& possible_moves)
{
    // Determine possible moves.
    int last_possible_count = possible_moves.move_count;
    position->determine_moves(color_sign, possible_moves);
    int move_count = possible_moves.move_count - last_possible_count;
    uint64_t nodes = 0;

    if(currently_evaluating_perft_depth == 1)
    {
        for(int i = last_possible_count; i < possible_moves.move_count; i++)
        {
            // Move count for debugging.
            if(depth == currently_evaluating_perft_depth-1)
            {
                std::string move_string = possible_moves.moves[i].to_string();
                std::cout << move_string << ": " << 1 << '\n';
            }   
        } 
    }

    // Base case.
    if(depth == 0)
    {
        possible_moves.move_count -= move_count;
        return move_count;
    }

    uint64_t key;
    // Make hash entries of position.
    if((currently_evaluating_perft_depth - depth) <= MAX_HASH_DEPTH)
    {
        key = hasher.calculate_zobrist_key(position, color_sign);
        uint64_t entry_node_count = transposition_table.get_entry_nodes(depth, key);
        // Read hash entry.
        if(entry_node_count != no_hash_entry)
        {
            // Position wes already evaluated in a different order.
            possible_moves.move_count -= move_count;
            return entry_node_count;
        }
    }

    for(int i = last_possible_count; i < possible_moves.move_count; i++)
    {
        // Do move.
        int move_index = i;
        
        position->do_move(&possible_moves.moves[move_index]);
        // Recursive call.
        uint64_t nodes_found = perft_test(position, depth-1, !color_sign, captures, checks, en_passants, possible_moves);
        nodes += nodes_found;
        // Undo move.

        position->undo_move(&possible_moves.moves[move_index]);

        // Move count for debugging.
        if(depth == currently_evaluating_perft_depth-1)
        {
            std::string move_string = possible_moves.moves[move_index].to_string();
            std::cout << move_string << ": " << nodes_found << '\n';
        }

        // Insert hash key.
        if((currently_evaluating_perft_depth - depth) <= MAX_HASH_DEPTH && (currently_evaluating_perft_depth - depth) >= MIN_HASH_DEPTH)
            transposition_table.insert_hash(depth, 0, 0, key, nodes);
    }
    // Return result.
    possible_moves.move_count -= move_count;
    return nodes;
}

// // Function to return the best found move.
// void Engine::best_move(Position* position, bool color_sign, int depth, Move& best_move)
// {
//     // For analysis.
//     int count = 0;
//     int zobrist_skips = 0;
//     clock_t timer = clock();

//     // Initialize
//     int alpha = -100;
//     int beta = 100;
//     float score = 0.f;
    
//     Move best_found;

//     // Find best move.
//     score = search(depth, alpha, beta, count, position, best_found, true, !color_sign, zobrist_skips, 0);

//     // Chech if engine was stopped due to time.
//     if(score == -999999)
//     {
//         return;
//     }

//     previous_score = score;

//     // Analysis.
//     timer = clock() - timer;
//     float elapsed_seconds = static_cast<float>(timer) / CLOCKS_PER_SEC;
    
//     // Output results.
//     std::cout << "Positions evaluated: " << count << "\n";
//     std::cout << "Score found was: " << score << "\n";
//     std::cout << "Average positions per second: " << count / elapsed_seconds << '\n';
//     std::cout << "Time taken: " << elapsed_seconds << "\n";
//     std::cout << "Nodes skipped thanks to mr. Zobrist: " << zobrist_skips << "\n";

//     // Check if a valid move was found. 64 is the signal that none was found.
//     assert(best_move.start_location != 64);
//     best_move = Move(best_found);
//     return;
// }

// float Engine::search(int current_depth, int alpha, int beta, int& position_count, Position* position, Move& best_move, bool top_level, bool maximizing, int& zobrist_skips, int depth_limit)
// {
//     // Initialize ==================================================================================================================

//     // Check if the time is up. If so, signal this to parent calls.
//     if(time_up)
//         return -999999;

//     // Make hash entries of position.
//     int hashf = hashfALPHA;
//     uint64_t key = hasher.calculate_zobrist_key(position, !maximizing);
//     int entry_key_value = transposition_table.read_hash_entry(alpha, beta, current_depth, key);

//     // Read hash entry.
//     if(entry_key_value != no_hash_entry && !top_level)
//     {
//         // Position wes already evaluated in a different order.
//         zobrist_skips++;
//         return entry_key_value;
//     }

//     // Count unique positions visited.
//     position_count++;

//     // End of depth, evaluate position.
//     if(current_depth <= depth_limit)
//     {
//         int val = evaluate_position(position);
//         transposition_table.insert_hash(current_depth, val, hashfEXACT, key);
//         return val;
//     }

//     // Determine possible moves.
//     std::vector<Move> possible_moves = position->determine_moves(!maximizing);

//     // No moves available means current player loses.
//     if(possible_moves.empty())
//         return maximizing ? -100 : 100;

//     sort_move_priority(possible_moves, position);

//     float eval = maximizing ? -100 : 100;

//     Move local_best_move = possible_moves.front();

//     bool first_node = true;

//     // Actual search. ==================================================================================================================
    
//     for(Move& move : possible_moves)
//     {
//         // Do move.
//         position->do_move(&move);
//         // Evaluate.
//         float score = search(current_depth - 1, alpha, beta, position_count, position, best_move, false, !maximizing, zobrist_skips, depth_limit);
//         // Undo.
//         position->undo_move(&move);
//         // Check if time is up.
//         if(score == -999999)
//             return -999999;

//         // Evaluate found score and edit alpha, beta, and best move accordingly.
//         if (maximizing)
//         {
//             if (score >= eval)
//             {
                
//                 eval = score;
//                 if (eval > alpha)
//                 {
//                     alpha = eval;
//                     local_best_move = Move(move);
//                     hashf = hashfEXACT;
//                 }
//             }
//             if (eval >= beta)
//             {
//                 transposition_table.insert_hash(current_depth, eval, hashfBETA, key);
//                 break;
//             }
//         }
//         else
//         {
//             if (score <= eval)
//             {
                
//                 eval = score;
//                 if (eval < beta)
//                 {
//                     beta = eval;
//                     local_best_move = Move(move);
//                     hashf = hashfEXACT;
//                 }
//             }
//             if (eval <= alpha)
//             {
//                 transposition_table.insert_hash(current_depth, eval, hashfALPHA, key);
//                 break;
//             }
//         }
//     }

//     if(top_level)
//         best_move = local_best_move;

//     transposition_table.insert_hash(current_depth, eval, hashf, key);
//     return eval;
// }

// void Engine::sort_move_priority(std::vector<Move>& moves, Position* position)
// {
//     for (Move& move : moves)
//     {
//         // Set priority groups based on properties.
//         if (move.is_capture(position)) 
//         {
//             move.priority_group = 1;
//         } else if (move.is_check(position)) 
//         {
//             move.priority_group = 2;
//         } else 
//         {
//             move.priority_group = 3;
//         }
//     }

//     // Sort vector according to priority groups.
//     std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
//         // Sort by priority_group first.
//         if (a.priority_group != b.priority_group) {
//             return a.priority_group < b.priority_group;
//         }

//         return a.evaluation > b.evaluation; 
//     });
// }


// float Engine::evaluate_piece_sum(Position* position, uint8_t color_sign)
// {
//     float points = 0.f;
//     for(int i = 0; i < 64; i++)
//     {
//         uint8_t piece = position->get_piece(i);
        
//         if(piece == EMPTY || piece > 5 != color_sign)
//             continue;

//         points += get_piece_value(piece);
//     }

//     return points;
// }

// float Engine::evaluate_position(Position* position)
// {
//     float total_eval = 0.f;

//     // Calculate difference in pieced value.
//     float black_points = evaluate_piece_sum(position, 1) * piece_value_weight;
//     float white_points = evaluate_piece_sum(position, 0) * piece_value_weight;

//     // Evaluate piece positions.
//     black_points += evaluate_square_bonus(position, 1) * square_bonus_weight;
//     white_points += evaluate_square_bonus(position, 0) * square_bonus_weight;
    
//     total_eval = white_points - black_points;
//     return total_eval;
// }

// float Engine::evaluate_square_bonus(Position* position, uint8_t color_sign)
// {   
//     float total = 0.f;
//     for(int i = 0; i < 64; i++)
//     {
//         uint8_t piece = position->get_piece(i);
//         if(color_sign != piece > 5 || piece == EMPTY)
//             continue;
//         else
//         {
//             int it = i;
//             if(color_sign)
//                 it = 63-i;
//             switch(piece)
//             {
//                 case B_PAWN:
//                 case W_PAWN:
//                     total += PAWN_BONUS[it];
//                     break;
//                 case B_KNIGHT:
//                 case W_KNIGHT:
//                     total += KNIGHT_BONUS[it];
//                     break;
//                 case B_BISHOP: 
//                 case W_BISHOP:
//                     total += BISHOP_BONUS[it];
//                     break;
//                 case B_ROOK:
//                 case W_ROOK:
//                     total += ROOK_BONUS[it];
//                     break;
//                 case B_QUEEN:
//                 case W_QUEEN:
//                     total += QUEEN_BONUS[it];
//                     break;
//                 case B_KING:
//                 case W_KING:
//                         total += KING_BONUS[it];
//                     break;
//                 default:
//                     break;
//             }
//         }
//     }
//     return total;
// }
