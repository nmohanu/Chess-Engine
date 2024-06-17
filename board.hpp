#include "util.hpp"
#include <iomanip>

struct Move;

struct Position
{
    void initialize();
    
    // Replace 4 bits by 4 new bits.
    void set_piece(uint8_t new_piece, uint8_t pos);

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos) const;

    // Do a move.
    void do_move(Move* move);

    // Check if king is under attack.
    bool king_under_attack(bool is_white);

    // Create attack board for specific piece.
    uint64_t make_attack_board(uint8_t x, uint8_t y);

    // Create attack board for a player.
    uint64_t color_attack_board(bool is_white);

    uint8_t get_piece_position(uint8_t piece);

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
    std::vector<Move*> determine_moves(bool is_white, Position &position) const;

    // Print board.
    void print() const;
    
    Position* position = new Position();
};

struct Move
{   
    Move(uint64_t start, uint64_t end) : start_location(start), end_location(end){}

    uint64_t start_location;
    uint64_t end_location;
    bool is_white;
};
