#include "board.hpp"


void Position::initialize()
{
    for(bool b : this->board_color)
    {
        b = false;
    }
    for(int i = 48; i < 64; i++)
    {
        this->board_color[i] = true;
    }

    // Set the pawns.
    for(int i = 8; i < 16; i++)
    {
        toggle_bit(pawns, i);
    }
    for(int i = 48; i < 56; i++)
    {
        toggle_bit(pawns, i);
    }

    // Set the other pieces.
    // Rooks.
    toggle_bit(rooks, 0);
    toggle_bit(rooks, 7);
    toggle_bit(rooks, 63);
    toggle_bit(rooks, 56);

    // Bishops.
    toggle_bit(bishops, 1);
    toggle_bit(bishops, 6);
    toggle_bit(bishops, 62);
    toggle_bit(bishops, 57);

    // Knights
    toggle_bit(knights, 2);
    toggle_bit(knights, 5);
    toggle_bit(knights, 61);
    toggle_bit(knights, 58);

    // Kings and queens
    toggle_bit(kings, 4);
    toggle_bit(queens, 3);
    toggle_bit(kings, 60);
    toggle_bit(queens, 59);
}

Board::Board()
{
    position->initialize();
}

Board::~Board()
{
    delete position;
}


void Board::print()
{
    for(int i = 0; i < 8; i++)
    {
        std::cout<< std::setw(5) << "=====";
    }
    std::cout << std::endl;
        
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            int pos = y * 8 + x;
            if(get_bit(position->pawns, pos))
            {
                std::cout << std::setw(5) << "| P |";
            }
            else if(get_bit(position->bishops, pos))
            {
                std::cout << std::setw(5) << "| B |";
            }
            else if(get_bit(position->rooks, pos))
            {
                std::cout << std::setw(5) << "| R |";
            }
            else if(get_bit(position->knights, pos))
            {
                std::cout << std::setw(5) << "| H |";
            }
            else if(get_bit(position->kings, pos))
            {
                std::cout << std::setw(5) << "| K |";
            }
            else if(get_bit(position->queens, pos))
            {
                std::cout << std::setw(5) << "| Q |";
            }
            else
            {
                std::cout<< std::setw(5) << "|   |";
            }
        }
        std::cout << std::endl;
        for(int i = 0; i < 8; i++)
        {
            std::cout<< std::setw(5) << "=====";
        }
        std::cout << std::endl;
    }
}