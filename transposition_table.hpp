#include "board.hpp"
#include <cstdint>
#include <unordered_map>
#include <optional>
#include <random>

#ifndef TT_HPP
#define TT_HPP

// Data structure for the transposition table.
typedef struct 
{
    uint64_t key = no_hash_entry;
    int depth;
    int flags;
    int score;
} tt;

struct TranspositionTable
{
    TranspositionTable() : transposition_table(hash_table_size) {
        clear_table(); // Ensure the table is initialized with invalid entries.
    }

    int read_hash_entry(int alpha, int beta, int depth, uint64_t key)
    {
        tt* hash_entry = &transposition_table[key % hash_table_size];

        // Check if position is correct.
        if(hash_entry->key != no_hash_entry && hash_entry->key == key)
        {
            // Make sure our depth is correct.
            if(hash_entry->depth >= depth)
            {
                if(hash_entry->flags == hashfEXACT)
                {
                    return hash_entry->score;
                }
                if(hash_entry->flags == hashfALPHA && hash_entry->score <= alpha)
                {
                    return alpha;
                }
                if(hash_entry->flags == hashfBETA && hash_entry->score >= beta)
                {
                    return beta;
                }
            }
        }
        
        // Does not exist.
        return no_hash_entry;
    }

    // Store hash entry in the table.
    void insert_hash(int depth, int score, int hash_flag, uint64_t key)
    {
        tt* hash_entry = &transposition_table[key % hash_table_size];

        hash_entry->depth = depth;
        hash_entry->flags = hash_flag;
        hash_entry->score = score;
        hash_entry->key = key;
    }

    // clear table.
    void clear_table()
    {
        for(int index = 0; index < transposition_table.size(); index++)
        {
            transposition_table[index].key = no_hash_entry;
            transposition_table[index].depth = 0;
            transposition_table[index].flags = 0;
            transposition_table[index].score = 0;
        }
    }

    // TT instance.
    std::vector<tt> transposition_table;
};

struct ZobristHash
{
    ZobristHash(){}

    void init_zobrist_keys();

    uint64_t calculate_zobrist_key(Position* position, uint8_t current_player_sign);

    void update_zobrist_hash(Move* move, Position* position, uint8_t current_player_sign, uint64_t& old_hash);

    uint64_t piece_keys[14][64];
    uint64_t enpassant_keys[64];
    uint64_t castle_keys[16];
    uint64_t side_key;
};

#endif