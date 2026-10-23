#include "ObstacleGenerator.h"

#include <iomanip>
#include <iostream>

int main() {
    VoxelGrid grid(20, 20, 10, 0.5);
    ObstacleGenerator::placeBox(grid, {3, 4, 0}, 4, 5, 6);
    ObstacleGenerator::generateRandom(grid, 0.1, 42,
                                     VoxelCoordinate{0, 0, 0},
                                     VoxelCoordinate{19, 19, 9});

    std::size_t occupied = 0;
    for (int z = 0; z < grid.depth(); ++z) {
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                occupied += grid.isOccupied(x, y, z) ? 1 : 0;
            }
        }
    }
    const auto total = static_cast<std::size_t>(grid.width())
        * grid.height() * grid.depth();
    std::cout << "Occupied voxels: " << occupied << " / " << total << '\n'
              << "Occupancy: " << std::fixed << std::setprecision(2)
              << 100.0 * static_cast<double>(occupied) / static_cast<double>(total)
              << "%\n";
    return 0;
}
