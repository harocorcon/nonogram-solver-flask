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

    std::vector<std::vector<std::vector<int>>> rowPatterns;  // [row][patternIndex][cell]   
    std::vector<std::vector<std::vector<int>>> colPatterns;  // [col][patternIndex][cell]   

    std::vector<int> rowPatternCounts;
    std::vector<int> colPatternCounts;
};

void clearBoard(Puzzle &puzzle) {
    std::cout << "Clearing board of size " << puzzle.rows << "x" << puzzle.cols << "...\n";
    for(int r = 0; r < puzzle.rows; r++) {
        for(int c = 0; c < puzzle.cols; c++){
            puzzle.board[r][c] = UNKNOWN;
        }
    }
}

int minimumSpaceRequired(const std::vector<int>& clues, int clueIndex) {
    int space = 0;

    for(int i = clueIndex; i < clues.size(); i++){
        space += clues[i];
    }

    // one gap between every remaining pair of clues
    int remainingClues = clues.size() - clueIndex;
    space += remainingClues - 1;

    return space;

}

void generatePatternsRecursive(
    int length,
    const std::vector<int>& clues,
    int clueIndex,
    int position,
    std::vector<int>& currentPattern,
    std::vector<std::vector<int>>& patterns
) {
    // Base case: if all clues have been placed
    if(clueIndex == clues.size()) {
        patterns.push_back(currentPattern);
        return;
    }

    int blockSize = clues[clueIndex];

    // Try placing this block at every possible position
    int minSpace = minimumSpaceRequired(clues, clueIndex);
    for(int start = position; start <= length - minSpace; start++) {
        // make sure the block fits with enough room for the remaining clues
        for( int i = 0; i < blockSize; i++) {
            currentPattern[start + i] = FILLED;
        }
        // move past this block and leave one empty cell
        int nextPosition = start + blockSize + 1;

        generatePatternsRecursive(length, clues, clueIndex + 1, nextPosition, currentPattern, patterns);

        // undo the placement
        for(int i = 0; i < blockSize; i++) {
            currentPattern[start + i] = UNKNOWN;
        }
    }
}

std::vector<std::vector<int>> generatePatterns(
    int length,
    const std::vector<int>& clues
)
{
    std::vector<std::vector<int>> patterns;
    
    std::vector<int> currentPattern(length, UNKNOWN);

    generatePatternsRecursive(length, clues, 0, 0, currentPattern, patterns);

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

    std::vector<int> clues = {2, 1};

    std::vector<std::vector<int>> patterns = generatePatterns(5, clues);

    for(const auto& pattern : patterns) {
        for (int cell : pattern) {
            std::cerr << cell << "-";
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

bool matchesBoard (std::vector<std::vector<int>> board, std::vector<std::vector<int>> pattern) {
    // only patterns compatible with the current board
    
    return true;
}

void testGenerateAllPatterns() {
    Puzzle puzzle;
    puzzle.rows = 5;
    puzzle.cols = 5;
    puzzle.board = std::vector<std::vector<int>>(puzzle.rows, std::vector<int>(puzzle.cols, UNKNOWN));

    puzzle.rowClues = {
        {2, 1},
        {1, 1, 1},
        {3, 1},
        {1, 2},
        {5}
    };
    puzzle.colClues = {
        {3, 1},
        {1, 2},
        {2, 1},
        {3},
        {2, 1}
    };

    std::cout << "Testing pattern generation " << std::endl;
    puzzle.rowPatterns.resize(puzzle.rows);
    for(int r = 0; r < puzzle.rows; r++) {
        puzzle.rowPatterns[r] = generatePatterns(puzzle.cols, puzzle.rowClues[r]);
        std::cout << "Row " << r << " patterns: " << puzzle.rowPatterns[r].size() << std::endl;
        for(int c : puzzle.rowClues[r]) {
            std::cout << c << " ";
        }
        std::cout << "-> " << puzzle.rowPatterns[r].size() << " patterns:" << std::endl;
    
        for(const auto& pattern : puzzle.rowPatterns[r]) {
            std::cout << " ";
            for(int cell : pattern) {
                if(cell == FILLED) {
                    std::cout << "#";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    
    // Columns
    puzzle.colPatterns.resize(puzzle.cols);
    for (int c = 0; c < puzzle.cols; c++) {
        puzzle.colPatterns[c] = generatePatterns(puzzle.rows, puzzle.colClues[c]);
        std::cout << "Col " << c << " clues: ";
        for (int r : puzzle.colClues[c]) std::cout << r << " ";
        std::cout << "-> " << puzzle.colPatterns[c].size() << " patterns:" << std::endl;
        for (const auto& p : puzzle.colPatterns[c]) {
            std::cout << "  ";
            for (int cell : p) {
                if (cell == FILLED) std::cout << "#";
                else std::cout << ".";
            }
            std::cout << std::endl;
        }
    }
}


int main(int argc, char* argv[]) {
    std::string action = argv[1];
    int rows = std::stoi(argv[2]);
    int cols = std::stoi(argv[3]);

    Puzzle puzzle = readPuzzle(rows, cols);
    
    if(action == "solve") {
        // solvePuzzle(puzzle);
        testGenerateAllPatterns();
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return 1;
    }

    printBoard(puzzle);
    
    return 0;
}

