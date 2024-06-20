#include "engine.hpp"

void ZobristHash::init_zobrist_keys()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    // Initialize piece keys for each piece type and each square
    for (int piece = 0b0; piece <= 0b1110; piece++) {
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_pieces[sq][piece] = dis(gen);
        }
    }

    // Initialize en passant keys for each file
    for (int file = 0; file < 8; ++file) {
        zobrist_en_passant_file[file] = dis(gen);
    }

    // Initialize key for current player turn (Black to move)
    zobrist_black_to_move = dis(gen);
}

uint64_t ZobristHash::calculate_zobrist_key(Position* position, uint8_t current_player_sign)
{
    uint64_t hash_key = 0b0;

    for (int sq = 0; sq < 64; ++sq) 
    {
        int piece = position->get_piece(sq);
        hash_key ^= zobrist_pieces[sq][piece];
    }

    // Hash en passant file
    if (position->en_passant != 0b11111111) 
    {
        uint8_t file = 0b0 | (position->en_passant & 0b00111111);
        hash_key ^= zobrist_en_passant_file[file];
    }

    // Hash current player.
    if (current_player_sign) 
    {
        hash_key ^= zobrist_black_to_move;
    }

    return hash_key;
}

void ZobristHash::update_zobrist_hash(Move* move, Position* position, uint8_t current_player_sign, uint64_t& old_hash)
{
    assert(move->move_bounds_valid());
    
    uint8_t from = move->start_location;
    uint8_t to = move->end_location;
    uint8_t captured_piece = position->get_piece(to);
    uint8_t moved_piece = position->get_piece(from);

    old_hash ^= zobrist_pieces[from][moved_piece];
    old_hash ^= zobrist_pieces[to][moved_piece];

    if (captured_piece != EMPTY) 
    {
        old_hash ^= zobrist_pieces[to][captured_piece];
    }

    // Update en passant file
    if (move->move_takes_an_passant) 
    {
        uint8_t captured_pawn_square = to + (current_player_sign? +8 : -8);
        uint8_t en_passant_file = captured_pawn_square % 8;
        old_hash ^= zobrist_en_passant_file[en_passant_file];
    }
    old_hash ^= zobrist_black_to_move;
}