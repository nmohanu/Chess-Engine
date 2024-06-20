#include "engine.hpp"
#include <SFML/Graphics.hpp>
#include <utility>
#include <iostream>

int main()
{
    Board* board = new Board();
    
    sf::Texture texture;
    if (!texture.loadFromFile("../../assets/sheet.png"))
    {
        return 1;
    }

    int checkpoint_count = 0;

    bool mouse_pressed = false;

    bool is_white_turn = true;

    Engine engine;

    float SCALE_FACTOR = 8.f;
    int SCREEN_WIDTH = 1080;
    int SCREEN_HEIGHT = 1080;

    uint8_t selected_piece;

    std::pair<int,int> clicked_square;
    std::pair<int,int> last_clicked_square;

    uint64_t move_board = 0b0;
    uint64_t reach_board = 0b0;

    std::vector<Move> possible_moves;
    possible_moves = board->position->determine_moves(!is_white_turn);

    sf::Vector2f offset((SCREEN_WIDTH - 16*SCALE_FACTOR*8)/2, (SCREEN_HEIGHT - 16*SCALE_FACTOR*8)/2);

    sf::Sprite square_white(texture);
    square_white.setTextureRect(sf::IntRect(0, 0, 16, 16));
    square_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite square_brown(texture);
    square_brown.setTextureRect(sf::IntRect(16, 0, 16, 16));
    square_brown.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite pawn_black(texture);
    pawn_black.setTextureRect(sf::IntRect(32, 0, 16, 16));
    pawn_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite pawn_white(texture);
    pawn_white.setTextureRect(sf::IntRect(48, 0, 16, 16));
    pawn_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite rook_black(texture);
    rook_black.setTextureRect(sf::IntRect(64, 0, 16, 16));
    rook_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite rook_white(texture);
    rook_white.setTextureRect(sf::IntRect(80, 0, 16, 16));
    rook_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite knight_black(texture);
    knight_black.setTextureRect(sf::IntRect(96, 0, 16, 16));
    knight_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite knight_white(texture);
    knight_white.setTextureRect(sf::IntRect(112, 0, 16, 16));
    knight_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite bishop_black(texture);
    bishop_black.setTextureRect(sf::IntRect(128, 0, 16, 16));
    bishop_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite bishop_white(texture);
    bishop_white.setTextureRect(sf::IntRect(144, 0, 16, 16));
    bishop_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite king_black(texture);
    king_black.setTextureRect(sf::IntRect(160, 0, 16, 16));
    king_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite king_white(texture);
    king_white.setTextureRect(sf::IntRect(176, 0, 16, 16));
    king_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite queen_black(texture);
    queen_black.setTextureRect(sf::IntRect(192, 0, 16, 16));
    queen_black.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite queen_white(texture);
    queen_white.setTextureRect(sf::IntRect(208, 0, 16, 16));
    queen_white.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::Sprite selection_square(texture);
    selection_square.setTextureRect(sf::IntRect(224, 0, 16, 16));
    selection_square.setScale(SCALE_FACTOR, SCALE_FACTOR);

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Chess Engine");

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        
        sf::Vector2i mouse_position;
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !mouse_pressed)
        {
            // Get clicked square coordinates.
            std::pair<int, int> new_square;
            mouse_position = sf::Mouse::getPosition(window);
            clicked_square.first = (mouse_position.x - offset.x) / (16 * SCALE_FACTOR);
            clicked_square.second = (mouse_position.y - offset.y) / (16 * SCALE_FACTOR);

            // Check if user is moving a piece.
            if(last_clicked_square != clicked_square)
            {
                selected_piece = board->position->get_piece(make_pos(clicked_square.first, clicked_square.second));

                move_board = board->position->make_move_board(clicked_square.first, clicked_square.second);
                reach_board = board->position->make_reach_board(clicked_square.first, clicked_square.second);
                if(selected_piece == 0)
                {
                    move_board = 0b0;
                    reach_board = 0b0;
                }
                
                for (Move move : possible_moves)
                {
                    if(move.start_location == make_pos(last_clicked_square.first, last_clicked_square.second) && move.end_location == make_pos(clicked_square.first, clicked_square.second))
                    {
                        board->position->do_move(&move);
                        is_white_turn = !is_white_turn;
                        possible_moves = board->position->determine_moves(!is_white_turn);
                        if(possible_moves.empty())
                        {
                            std::cout << "Player " << !is_white_turn << " wins! \n";
                        }
                        break;
                    }
                }
            }
            mouse_pressed = true;
        }
        else
        {
            mouse_pressed = false;
        }
        
        if(!is_white_turn)
        {
            bool color_sign = !is_white_turn;
            float alpha = MIN_EVAL;  
            float beta = MAX_EVAL; 
            Move best_move = engine.best_move(board->position, color_sign, 4);
            board->position->do_move(&best_move);
            is_white_turn = !is_white_turn;
            possible_moves = board->position->determine_moves(!is_white_turn);
        }

        // std::cout << clicked_square.first << " " << clicked_square.second << '\n';
        // std::cout << mouse_position.x << " " << mouse_position.y << '\n';

        window.clear();

        // Draw stuff.
        bool square_color = 1;
        for(int y = 0; y < 8; y++)
        {
            for(int x = 0; x < 8; x++)
            {
                int pos = y * 8 + x;

                sf::Sprite* piece_to_draw = nullptr;

                sf::Vector2f print_position(x*16*SCALE_FACTOR + offset.x, y*16*SCALE_FACTOR + offset.y);

                // Draw the board.
                if(square_color)
                {
                    square_white.setPosition(sf::Vector2f(print_position));
                    window.draw(square_white);
                }
                else
                {
                    square_brown.setPosition(sf::Vector2f(print_position));
                    window.draw(square_brown);
                }

                // Flip color at end of row.
                if(x != 7)
                    square_color = !square_color;

                // Draw the pieces.
                uint8_t piece_type = board->position->get_piece(pos);
                switch (piece_type) 
                {
                    case B_PAWN:
                        piece_to_draw = &pawn_black;
                        break;
                    case W_PAWN:
                        piece_to_draw = &pawn_white;
                        break;
                    case B_KING:
                        piece_to_draw = &king_black;
                        break;
                    case W_KING:
                        piece_to_draw = &king_white;
                        break;
                    case B_QUEEN:
                        piece_to_draw = &queen_black;
                        break;
                    case W_QUEEN:
                        piece_to_draw = &queen_white;
                        break;
                    case B_ROOK:
                        piece_to_draw = &rook_black;
                        break;
                    case W_ROOK:
                        piece_to_draw = &rook_white;
                        break;
                    case B_BISHOP:
                        piece_to_draw = &bishop_black;
                        break;
                    case W_BISHOP:
                        piece_to_draw = &bishop_white;
                        break;
                    case B_KNIGHT:
                        piece_to_draw = &knight_black;
                        break;
                    case W_KNIGHT:
                        piece_to_draw = &knight_white;
                        break;
                    default:
                        break;
                }
                

                // Draw the piece on our iteration coordinates.
                if(piece_to_draw != nullptr)
                {
                    piece_to_draw->setPosition(sf::Vector2f(print_position));
                    window.draw(*piece_to_draw);
                }
                
                if(x == clicked_square.first && y == clicked_square.second)
                {
                    selection_square.setPosition(sf::Vector2f(print_position));
                    window.draw(selection_square);
                }

                uint8_t piece_at_pos = board->position->get_piece(pos);
                if((get_bit_64(move_board, pos) || get_bit_64(reach_board, pos) && get_color(piece_at_pos) == is_white_turn && piece_at_pos != 0) && get_color(selected_piece) != is_white_turn)
                {
                    selection_square.setPosition(sf::Vector2f(print_position));
                    window.draw(selection_square);
                }
            }
        }
        
        last_clicked_square = clicked_square;
        window.display();
    }

    return 0;
}