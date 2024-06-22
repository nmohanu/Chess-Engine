#include "board.hpp"
#include <cstdint>
#include <unordered_map>
#include <optional>
#include <random>

struct TranspositionTableEntry {
    uint64_t hash;
    int depth;
    float score;
};

struct TranspositionTable {
    std::unordered_map<uint64_t, TranspositionTableEntry> table;

    bool contains(uint64_t hash) {
        return table.find(hash) != table.end();
    }

    void insert(uint64_t hash, int depth, float score) {
        table[hash] = {hash, depth, score};
    }

    std::optional<TranspositionTableEntry> get(uint64_t hash) {
        auto it = table.find(hash);
        if (it != table.end()) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }

    void clear() {
        table.clear();
    }
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