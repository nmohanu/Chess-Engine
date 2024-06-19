#include "util.hpp"
#include <iomanip>

struct Move;

struct Position
{
    void initialize();

    Position();

    ~Position();

    Position(const Position& other);
    
    // Replace 4 bits by 4 new bits.
    void set_piece(uint8_t new_piece, uint8_t pos);

    // Get piece in position x y.
    uint8_t get_piece(uint8_t pos) const;

    // Do a move.
    void do_move(Move* move);

    // Check if king is under attack.
    bool king_under_attack(bool color_sign);

    // Create attack board for specific piece.
    uint64_t make_reach_board(uint8_t x, uint8_t y);

    uint64_t make_move_board(uint8_t, uint8_t);

    // Create attack board for a player.
    uint64_t color_reach_board(bool color_sign);

    // Get position of a piece.
    uint8_t get_piece_position(uint8_t piece);

    // Determine possible moves.
    std::vector<Move> determine_moves(bool color_sign);

    // false = black, true = white.
    uint64_t first_16;
    uint64_t second_16;
    uint64_t third_16;
    uint64_t fourth_16;

    Move* last_move = nullptr;
};

class Board 
{
public:
    Board();

    ~Board();    
    
    Position* position = nullptr;
};

struct Move
{   
    Move();
    Move(const Move& other);
    Move(uint8_t start, uint8_t end) : start_location(start), end_location(end) {}

    bool is_check(Position* position) const;
    bool is_capture(Position* position) const;
    float capture_value(Position* position) const;

    uint8_t start_location;
    uint8_t end_location;
};

struct MoveVec
{
    void insert(Move* move);
    void insert(uint8_t start, uint8_t end);
    bool exists(Move* move);
    bool exists(uint8_t start, uint8_t end);
    void remove(Move* move);
    void remove(uint8_t start, uint8_t end);
};
