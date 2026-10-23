#include "VoxelGrid.h"

#include <iostream>

int main() {
    VoxelGrid grid(20, 20, 10, 0.5);
    grid.setOccupied(2, 3, 4);
    grid.setOccupied(10, 10, 5);
    grid.setOccupied(19, 19, 9);

    std::cout << std::boolalpha
              << "Occupied (2, 3, 4): " << grid.isOccupied(2, 3, 4) << '\n'
              << "Occupied (10, 10, 5): " << grid.isOccupied(10, 10, 5) << '\n'
              << "Occupied (19, 19, 9): " << grid.isOccupied(19, 19, 9) << '\n'
              << "Occupied (0, 0, 0): " << grid.isOccupied(0, 0, 0) << '\n'
              << "Valid (19, 19, 9): " << grid.isValid(19, 19, 9) << '\n'
              << "Valid (20, 0, 0): " << grid.isValid(20, 0, 0) << '\n'
              << "Valid (-1, 0, 0): " << grid.isValid(-1, 0, 0) << '\n';
    return 0;
}
