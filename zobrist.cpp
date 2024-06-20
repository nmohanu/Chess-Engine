#include "engine.hpp"

void ZobristHash::init_zobrist_keys()
{
    // Initialize piece keys for each piece type and each square
    for (int piece = NONE; piece <= KING; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_pieces[sq][piece] = rand();
        }
    }

    // Initialize en passant keys for each file
    for (int file = 0; file < 8; ++file) {
        zobrist_en_passant_file[file] = rand();
    }

    // Initialize key for current player turn (Black to move)
    zobrist_black_to_move = rand();
}

uint64_t ZobristHash::calculate_zobrist_key(Position* position)
{
    uint64_t hash_key = 0;
    for (int sq = 0; sq < 64; ++sq) {
        int piece = position->get_piece(sq);
        if (piece != EMPTY) {
            hash_key ^= zobrist_pieces[sq][piece];
        }
    }
}