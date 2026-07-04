#include <iostream>
#include <vector>

const int UNKNOWN = 0;
const int FILLED = 1;
const int CROSSED = -1;

struct Puzzle {
    int rows;
    int cols;

    std::vector<std::vector<int>> rowClues;
    std::vector<std::vector<int>> colClues;
    std::vector<std::vector<int>> board;
};

void clearBoard(int rows, int cols) {
    for(int r = 0; r < rows; r++) {
        
        for(int c = 0; c < cols; c++){
            std::cout << "0 ";
        }
        std::cout << "\n";
    }
}

void solvePuzzle(int rows, int cols) {
    for(int r = 0; r < rows; r++) {
        
        for(int c = 0; c < cols; c++){
            std::cout << "1 ";
        }
        std::cout << "\n";
    }
}

void printBoard(const Puzzle& puzzle)
{
    for(const auto& row : puzzle.board)
    {
        for(int cell : row)
        {
            std::cout << cell << " ";
        }

        std::cout << "\n";
    }
}

void printRowClues(const Puzzle& puzzle)
{
    std::cout << "Row Clues:\n";
    for(const auto& clues : puzzle.rowClues)
    {
        for(int clue : clues)
        {
            std::cout << clue << " ";
        }
        std::cout << "\n";
    }
}

void printColumnClues(const Puzzle& puzzle)
{
    std::cout << "Column Clues:\n";
    for(const auto& clues : puzzle.colClues)
    {
        for(int clue : clues)
        {
            std::cout << clue << " ";
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    // std::string action = argv[1];
    // int rows = std::stoi(argv[2]);
    // int cols = std::stoi(argv[3]);

    // if(action == "clear") {
    //     clearBoard(rows, cols);
    // } else if(action == "solve") {
    //     solvePuzzle(rows, cols);
    // } else {
    //     std::cerr << "Unknown action: " << action << std::endl;
    //     return 1;
    // }   

    Puzzle puzzle;
    puzzle.rows = 5;
    puzzle.cols = 5;

    puzzle.rowClues = {
        {1},
        {3},
        {1,1},
        {5},
        {2}
    };

    puzzle.colClues = {
       {2},
        {1,1},
        {3},
        {1},
        {2}
    };

    puzzle.board = std::vector<std::vector<int>>(
        puzzle.rows,
        std::vector<int>(puzzle.cols, UNKNOWN)
    );

    std::cout << "Initial board state:\n";
    printBoard(puzzle);
    printRowClues(puzzle);
    printColumnClues(puzzle);
    
    return 0;
}