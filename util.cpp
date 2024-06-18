#include "util.hpp"
#include <bitset>

void toggle_bit_on(uint64_t &num, uint64_t pos)
{
    uint64_t mask = 1ULL << (63 - pos);
    num |= mask;
}

bool get_bit(uint8_t num, uint8_t pos)
{
    uint8_t mask = 1ULL << (7 - pos);
    return (num & mask) != 0;
}

bool get_bit_64(uint64_t num, uint64_t pos)
{
    uint64_t mask = 1ULL << (63 - pos);
    return (num & mask) != 0;
}

int make_pos(int x, int y)
{
    return 8*y+x;
}

bool get_color(uint8_t piece)
{
    return get_bit(piece, 0);
}

void print_binary(uint64_t num)
{
    std::cout << "Binary representation: ";
    std::cout << std::bitset<sizeof(num) * 8>(num) << std::endl;
}