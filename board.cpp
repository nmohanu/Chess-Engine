#include "board.hpp"



void Position::initialize()
{
    this->first_16 = FIRST_16_SQUARES;
    this->second_16 = SECOND_16_SQUARES;
    this->third_16 = THIRD_16_SQUARES;
    this->fourth_16 = FOURTH_16_SQUARES;
}

Board::Board()
{
    position->initialize();
}

Board::~Board()
{
    delete position;
}

// Get the piece on position x, y.
uint8_t Board::get_piece(uint8_t pos)
{
    uint8_t piece;
    if (pos < 16)
    {
        piece = this->position->first_16 >> (15 - pos)*4;
    }
    else if (pos < 32)
    {
        piece = this->position->second_16 >> (15 - (pos - 16))*4;
    }
    else if (pos < 48)
    {
        piece = this->position->third_16 >> (15 - (pos - 32))*4;
    }
    else
    {
        piece = this->position->fourth_16 >> (15 - (pos - 48))*4;
    }

    uint8_t mask = 0b00001111;
    piece &= mask;
    return piece;
}


void Board::print()
{
    // for(int i = 0; i < 8; i++)
    // {
    //     std::cout<< std::setw(5) << "=====";
    // }
    // std::cout << std::endl;
        
    // for(int y = 0; y < 8; y++)
    // {
    //     for(int x = 0; x < 8; x++)
    //     {
    //         int pos = y * 8 + x;
    //         if(get_bit(position->pawns, pos))
    //         {
    //             std::cout << std::setw(5) << "| P |";
    //         }
    //         else if(get_bit(position->bishops, pos))
    //         {
    //             std::cout << std::setw(5) << "| B |";
    //         }
    //         else if(get_bit(position->rooks, pos))
    //         {
    //             std::cout << std::setw(5) << "| R |";
    //         }
    //         else if(get_bit(position->knights, pos))
    //         {
    //             std::cout << std::setw(5) << "| H |";
    //         }
    //         else if(get_bit(position->kings, pos))
    //         {
    //             std::cout << std::setw(5) << "| K |";
    //         }
    //         else if(get_bit(position->queens, pos))
    //         {
    //             std::cout << std::setw(5) << "| Q |";
    //         }
    //         else
    //         {
    //             std::cout<< std::setw(5) << "|   |";
    //         }
    //     }
    //     std::cout << std::endl;
    //     for(int i = 0; i < 8; i++)
    //     {
    //         std::cout<< std::setw(5) << "=====";
    //     }
    //     std::cout << std::endl;
    // }
}