#include "engine.hpp"
#include <SFML/Graphics.hpp>

int main()
{
    Board* board = new Board();
    board->print();

    sf::Texture texture;
    if (!texture.loadFromFile("../../assets/sheet.png"))
    {
        return 1;
    }

    float SCALE_FACTOR = 6.f;
    int SCREEN_WIDTH = 1080;
    int SCREEN_HEIGHT = 1080;

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

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Chess Engine");

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        
        bool color = 1;

        window.clear();

        // Draw stuff.

        for(int y = 0; y < 8; y++)
        {
            for(int x = 0; x < 8; x++)
            {
                int pos = y * 8 + x;

                sf::Vector2f print_position(x*16*SCALE_FACTOR + offset.x, y*16*SCALE_FACTOR + offset.y);

                // Draw the board.
                if(color)
                {
                    square_white.setPosition(sf::Vector2f(print_position));
                    window.draw(square_white);
                }
                else
                {
                    square_brown.setPosition(sf::Vector2f(print_position));
                    window.draw(square_brown);
                }
                if(x != 7)
                    color = !color;

                // Draw the pieces.
                if (get_bit(board->position->pawns, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        pawn_white.setPosition(sf::Vector2f(print_position));
                        window.draw(pawn_white);
                    }
                    else
                    {
                        // Black piece
                        pawn_black.setPosition(sf::Vector2f(print_position));
                        window.draw(pawn_black);
                    }
                }
                else if (get_bit(board->position->bishops, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        bishop_white.setPosition(sf::Vector2f(print_position));
                        window.draw(bishop_white);
                    }
                    else
                    {
                        // Black piece
                        bishop_black.setPosition(sf::Vector2f(print_position));
                        window.draw(bishop_black);
                    }
                }
                else if (get_bit(board->position->rooks, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        rook_white.setPosition(sf::Vector2f(print_position));
                        window.draw(rook_white);
                    }
                    else
                    {
                        // Black piece
                        rook_black.setPosition(sf::Vector2f(print_position));
                        window.draw(rook_black);
                    }
                }
                else if (get_bit(board->position->knights, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        knight_white.setPosition(sf::Vector2f(print_position));
                        window.draw(knight_white);
                    }
                    else
                    {
                        // Black piece
                        knight_black.setPosition(sf::Vector2f(print_position));
                        window.draw(knight_black);
                    }
                }
                else if (get_bit(board->position->kings, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        king_white.setPosition(sf::Vector2f(print_position));
                        window.draw(king_white);
                    }
                    else
                    {
                        // Black piece
                        king_black.setPosition(sf::Vector2f(print_position));
                        window.draw(king_black);
                    }
                }
                else if (get_bit(board->position->queens, pos))
                {
                    if (get_bit(board->position->board_color, pos))
                    {
                        // White piece
                        queen_white.setPosition(sf::Vector2f(print_position));
                        window.draw(queen_white);
                    }
                    else
                    {
                        // Black piece
                        queen_black.setPosition(sf::Vector2f(print_position));
                        window.draw(queen_black);
                    }
                }
            }
        }
        
        
        window.display();
    }

    return 0;
}