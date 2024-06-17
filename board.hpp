#include "util.hpp"
#include <iomanip>

struct Move;

struct Position
{
    void initialize();
    
    // false = black, true = white.
    uint64_t first_16;
    uint64_t second_16;
    uint64_t third_16;
    uint64_t fourth_16;
};

class Board 
{
public:
    Board();

    ~Board();

    // Determine possible moves.
    std::vector<Move> determine_moves(bool is_white, Position &position);

    // Do a move.
    void do_move(Move &move, Position &position);

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos);

    // Print board.
    void print();
    
    Position* position = new Position();
};

struct Move
{   
    Move(uint64_t start, uint64_t end) : start_location(start), end_location(end){}

    uint64_t start_location;
    uint64_t end_location;
    bool is_white;
};
