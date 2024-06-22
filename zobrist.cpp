#include "transposition_table.hpp"

void ZobristHash::init_zobrist_keys()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    // // Pieces.
    for (uint8_t piece = 0b0; piece <= 0b1110; piece++)
    {
        // Squares.
        for(int square = 0; square < 64; square++)
        {
            uint64_t random_number = dis(gen);
            // Generate random keys.
            piece_keys[piece][square] = random_number;
        }
    }

    // En passant keys.
    for(int square = 0; square < 64; square++)
    {
        uint64_t random_number = dis(gen);
        enpassant_keys[square] = random_number;
    }

    // Side key.
    side_key = dis(gen);

    for(int i = 0; i < 16; i++)
    {
        castle_keys[i] = dis(gen);
    }
}

uint64_t ZobristHash::calculate_zobrist_key(Position* position, uint8_t current_player_sign)
{
    uint64_t key = 0b0;

    for (uint8_t i = 0b0; i < 64; i++)
    {
        uint8_t piece = position->get_piece(i);
        key ^= piece_keys[piece][i];
    }

    if(position->en_passant !=  0b11111111)
    {
        key ^= enpassant_keys[position->en_passant];
    }

    key ^= castle_keys[position->casling_rights];

    if(current_player_sign == 1)
    {
        key ^= side_key;
    }
    return key;
}

void ZobristHash::update_zobrist_hash(Move* move, Position* position, uint8_t current_player_sign, uint64_t& old_hash)
{

}