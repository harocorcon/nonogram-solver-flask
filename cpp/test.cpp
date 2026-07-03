#include <iostream>
#include <vector>

std::vector<std::vector<int>> rowClues;
std::vector<std::vector<int>> board;

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

    board = {
    {0, 1, 0},
    {0, 1, 0},
    {0, 0, 0}
    };

    std::cout << "Initial board state:\n";
    for(const auto& row : board) {
        for(int cell : row) {
            std::cout << cell << " ";
        }
        std::cout << "\n";
    }
    
    return 0;
}