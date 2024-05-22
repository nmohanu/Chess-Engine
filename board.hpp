#include "util.hpp"
#include <iomanip>

class Position
{
public:
    void initialize();
    
    // false = black, true = white.
    bool board_color[64];

    // Represent the board for each piece.
    uint64_t pawns = 0;
    uint64_t rooks = 0;
    uint64_t bishops = 0;
    uint64_t knights = 0;
    uint64_t kings = 0;
    uint64_t queens = 0;
};

class Board 
{
public:
    Board();

    // Print board.
    void print();
private:
    Position* position = new Position();
};

