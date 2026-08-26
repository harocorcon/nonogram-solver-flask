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

std::vector<std::vector<int>> generatePatterns(
    int length,
    const std::vector<int>& clues
)
{
    std::vector<std::vector<int>> patterns;
    // Implementation for generating patterns
    int blockSize = clues[0];

    int numberOfPositions = length - blockSize + 1;

    for(int start = 0; start < numberOfPositions; start++) {
        std::vector<int> pattern(length, UNKNOWN);

        for(int i = 0; i < blockSize; i++) {
            pattern[start + i] = FILLED;
        }

        patterns.push_back(pattern);
    }

    return patterns;
}

void solvePuzzle(Puzzle& puzzle) {
    for (int r = 0; r < puzzle.rows; r++)
    {
        for (int c = 0; c < puzzle.cols; c++)
        {
            puzzle.board[r][c] = FILLED;
        }
    }

    std::vector<int> clue = {3};

    std::vector<std::vector<int>> patterns = generatePatterns(5, clue);

    for(const auto& pattern : patterns) {
        for (int cell : pattern) {
            std::cerr << cell << " ";
        }
        std::cerr << "\n";
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

Puzzle readPuzzle(int rows, int cols) {
    Puzzle puzzle;
    puzzle.rows = rows;
    puzzle.cols = cols;
    puzzle.board.resize(rows, std::vector<int>(cols, UNKNOWN));

    std::string line;
    std::getline(std::cin, line);
    
    for (int r = 0; r < rows; r++){
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::vector<int> clue;

        int value;

        while (ss >> value)
        {
            clue.push_back(value);
        }

        puzzle.rowClues.push_back(clue);
    }

    for (int c = 0; c < cols; c++) {
        std::string line;
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::vector<int> clue;

        int value;

        while (ss >> value)
        {
            clue.push_back(value);
        }

        puzzle.colClues.push_back(clue);
    }

    puzzle.board = std::vector<std::vector<int>>(
        puzzle.rows,
        std::vector<int>(puzzle.cols, UNKNOWN)
    );   
    
    return puzzle;
}


int main(int argc, char* argv[]) {
    std::string action = argv[1];
    int rows = std::stoi(argv[2]);
    int cols = std::stoi(argv[3]);

    Puzzle puzzle = readPuzzle(rows, cols);

    if(action == "solve") {
        solvePuzzle(puzzle);
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return 1;
    }

    printBoard(puzzle);
    
    return 0;
}

