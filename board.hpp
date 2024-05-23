#include "util.hpp"
#include <iomanip>

class Position
{
public:
    void initialize();
    
    // false = black, true = white.
    uint64_t board_color = 0;

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

    ~Board();

    // Print board.
    void print();
    
    Position* position = new Position();
};

class Move
{   
    Move(uint64_t start, uint64_t end, bool is_white) : start_location(start), end_location(end), is_white(is_white)
    {
    }

    uint64_t start_location;
    uint64_t end_location;
    bool is_white;
};
