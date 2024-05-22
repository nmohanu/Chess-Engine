#include "util.hpp"
void toggle_bit(uint64_t &num, uint64_t pos)
{
    uint64_t mask = 1ULL << (63 - pos);
    num ^= mask;
}

bool get_bit(uint64_t num, uint64_t pos)
{
    uint64_t mask = 1ULL << (63 - pos);
    return (num & mask) != 0;
}