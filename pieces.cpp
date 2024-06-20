#include "board.hpp"

uint64_t Position::get_pawn_reach(uint8_t x, uint8_t y, bool is_white) {
    uint64_t attack_board = 0b0;

    if (is_white) {
        if (x + 1 <= 7 && y - 1 >= 0)
            toggle_bit_on(attack_board, make_pos(x + 1, y - 1));
        if (x - 1 >= 0 && y - 1 >= 0)
            toggle_bit_on(attack_board, make_pos(x - 1, y - 1));
    } else {
        if (x + 1 <= 7 && y + 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x + 1, y + 1));
        if (x - 1 >= 0 && y + 1 <= 7)
            toggle_bit_on(attack_board, make_pos(x - 1, y + 1));
    }

    return attack_board;
}

uint64_t Position::get_king_reach(uint8_t x, uint8_t y) {
    uint64_t attack_board = 0b0;
    int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int new_x = x + offsets[i][0];
        int new_y = y + offsets[i][1];
        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            int pos = make_pos(new_x, new_y);
            toggle_bit_on(attack_board, pos);
        }
    }

    return attack_board;
}

uint64_t Position::get_bishop_reach(uint8_t x, uint8_t y) {
    uint64_t attack_board = 0b0;
    int xDeltas[] = {-1, 1, -1, 1};
    int yDeltas[] = {-1, -1, 1, 1};

    for (int dir = 0; dir < 4; ++dir) {
        int xIt = x + xDeltas[dir];
        int yIt = y + yDeltas[dir];

        while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            toggle_bit_on(attack_board, pos_to_check);

            if (get_piece(pos_to_check) != 0)
                break;

            xIt += xDeltas[dir];
            yIt += yDeltas[dir];
        }
    }

    return attack_board;
}

uint64_t Position::get_knight_reach(uint8_t x, uint8_t y) {
    uint64_t attack_board = 0b0;
    int knight_moves[8][2] = {
        { 1,  2}, { 1, -2}, {-1,  2}, {-1, -2},
        { 2,  1}, { 2, -1}, {-2,  1}, {-2, -1}
    };

    for (int i = 0; i < 8; ++i) {
        int new_x = x + knight_moves[i][0];
        int new_y = y + knight_moves[i][1];
        if (new_x >= 0 && new_x <= 7 && new_y >= 0 && new_y <= 7) {
            toggle_bit_on(attack_board, make_pos(new_x, new_y));
        }
    }

    return attack_board;
}

uint64_t Position::get_rook_reach(uint8_t x, uint8_t y) {
    uint64_t attack_board = 0b0;
    int xDeltas[] = {0, 0, -1, 1};
    int yDeltas[] = {-1, 1, 0, 0};

    for (int dir = 0; dir < 4; ++dir) {
        int xIt = x + xDeltas[dir];
        int yIt = y + yDeltas[dir];

        while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
            uint8_t pos_to_check = make_pos(xIt, yIt);
            toggle_bit_on(attack_board, pos_to_check);

            if (get_piece(pos_to_check) != 0)
                break;

            xIt += xDeltas[dir];
            yIt += yDeltas[dir];
        }
    }

    return attack_board;
}

uint64_t Position::get_queen_reach(uint8_t x, uint8_t y) {
    return get_bishop_reach(x, y) | get_rook_reach(x, y);
}

// Moving logic: ================================================

uint64_t Position::get_pawn_move(uint8_t x, uint8_t y, bool is_white) {
    uint64_t move_board = 0b0;

    if (is_white) {
        if (get_piece(make_pos(x, y-1)) == 0) {
            if (y == 6 && get_piece(make_pos(x, y-2)) == 0) {
                toggle_bit_on(move_board, make_pos(x, y-2));
            }
            toggle_bit_on(move_board, make_pos(x, y-1));
        }
    } else {
        if (get_piece(make_pos(x, y+1)) == 0) {
            if (y == 1 && get_piece(make_pos(x, y+2)) == 0) {
                toggle_bit_on(move_board, make_pos(x, y+2));
            }
            toggle_bit_on(move_board, make_pos(x, y+1));
        }
    }

    return move_board;
}

uint64_t Position::get_king_move(uint8_t x, uint8_t y) {
    uint64_t move_board = 0b0;
    int offsets[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        int new_x = x + offsets[i][0];
        int new_y = y + offsets[i][1];
        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            if (this->get_piece(make_pos(new_x, new_y)) == 0)
                toggle_bit_on(move_board, make_pos(new_x, new_y));
        }
    }

    return move_board;
}

uint64_t Position::get_bishop_move(uint8_t x, uint8_t y) {
    uint64_t move_board = 0b0;
    int xDeltas[] = {-1, 1, -1, 1};
    int yDeltas[] = {-1, -1, 1, 1};

    for (int dir = 0; dir < 4; ++dir) {
        int xIt = x + xDeltas[dir];
        int yIt = y + yDeltas[dir];

        while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
            uint8_t pos_to_check = make_pos(xIt, yIt);

            if (get_piece(pos_to_check) == 0)
                toggle_bit_on(move_board, pos_to_check);

            if (get_piece(pos_to_check) != 0)
                break;

            xIt += xDeltas[dir];
            yIt += yDeltas[dir];
        }
    }

    return move_board;
}

uint64_t Position::get_knight_move(uint8_t x, uint8_t y) {
    uint64_t move_board = 0b0;
    int offsets[8][2] = {
        { 1,  2}, { 1, -2}, {-1,  2}, {-1, -2},
        { 2,  1}, { 2, -1}, {-2,  1}, {-2, -1}
    };

    for (int i = 0; i < 8; ++i) {
        int new_x = x + offsets[i][0];
        int new_y = y + offsets[i][1];
        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            if (this->get_piece(make_pos(new_x, new_y)) == 0)
                toggle_bit_on(move_board, make_pos(new_x, new_y));
        }
    }

    return move_board;
}

uint64_t Position::get_rook_move(uint8_t x, uint8_t y) {
    uint64_t move_board = 0b0;
    int xDeltas[] = {0, 0, -1, 1};
    int yDeltas[] = {-1, 1, 0, 0};

    for (int dir = 0; dir < 4; ++dir) {
        int xIt = x + xDeltas[dir];
        int yIt = y + yDeltas[dir];

        while (xIt >= 0 && yIt >= 0 && xIt < 8 && yIt < 8) {
            uint8_t pos_to_check = make_pos(xIt, yIt);

            if (get_piece(pos_to_check) == 0)
                toggle_bit_on(move_board, pos_to_check);

            if (get_piece(pos_to_check) != 0)
                break;

            xIt += xDeltas[dir];
            yIt += yDeltas[dir];
        }
    }

    return move_board;
}

uint64_t Position::get_queen_move(uint8_t x, uint8_t y) {
    return get_bishop_move(x, y) | get_rook_move(x, y);
}