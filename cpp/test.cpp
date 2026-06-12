#include <iostream>

void clearBoard(int rows, int cols) {
    for(int r = 0; r < rows; r++) {
        
        for(int c = 0; c < cols; c++){
            std::cout << "0";
        }
        std::cout << "\n";
    }
}

int main() {
    clearBoard(10, 10);

    return 0;
}