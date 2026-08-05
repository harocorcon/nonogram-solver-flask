#include <iostream>
#include <vector>
#include <sstream>
#include <string>

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

void clearBoard(Puzzle &puzzle) {
    std::cout << "Clearing board of size " << puzzle.rows << "x" << puzzle.cols << "...\n";
    for(int r = 0; r < puzzle.rows; r++) {
        for(int c = 0; c < puzzle.cols; c++){
            puzzle.board[r][c] = UNKNOWN;
        }
    }
}

void solvePuzzle(const Puzzle& puzzle) {
    for(int r = 0; r < puzzle.rows; r++) {
        for(int c = 0; c < puzzle.cols; c++){
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

void printClues(const Puzzle& puzzle, bool isRowClues)
{
    const auto& clues = isRowClues ? puzzle.rowClues : puzzle.colClues;

    for(int i = 0; i < clues.size(); ++i)
    {
        std::cout << (isRowClues ? "Row " : "Col ") << i + 1 << ": ";
        for(int clue : clues[i])
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

void readRowClues(Puzzle& puzzle) {
    std::string line;
    for(int r = 0; r < puzzle.rows; ++r) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        int clue;
        std::vector<int> clues;
        while(iss >> clue) {
            clues.push_back(clue);
        }
        puzzle.rowClues.push_back(clues);
    }
}

int main(int argc, char* argv[]) {
    Puzzle puzzle;
    std::string action = argv[1];
    puzzle.rows = std::stoi(argv[2]);
    puzzle.cols = std::stoi(argv[3]);
    puzzle.board.resize(puzzle.rows, std::vector<int>(puzzle.cols, UNKNOWN));
    
    if(action == "solve") {
        solvePuzzle(puzzle);
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return 1;
    }
    
    return 0;
}

