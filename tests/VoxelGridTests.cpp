#include "VoxelGrid.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Exception, typename Function>
void expectThrows(Function action) {
    try {
        action();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("Expected exception was not thrown");
}

void testInitializationAndUpdates() {
    VoxelGrid grid(3, 4, 2, 0.5);
    for (int z = 0; z < 2; ++z) {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 3; ++x) {
                require(!grid.isOccupied(x, y, z), "Voxel must start free");
            }
        }
    }
    grid.setOccupied(1, 2, 1);
    require(grid.isOccupied(1, 2, 1), "Default update must occupy voxel");
    grid.setOccupied(1, 2, 1, false);
    require(!grid.isOccupied(1, 2, 1), "Voxel must become free");
    grid.setOccupied(1, 2, 1, true);
    grid.setOccupied(1, 2, 1, true);
    require(grid.isOccupied(1, 2, 1), "Repeated update must be stable");
}

void testCoordinates() {
    VoxelGrid grid(3, 4, 2, 1.0);
    for (int x : {0, 2}) {
        for (int y : {0, 3}) {
            for (int z : {0, 1}) {
                require(grid.isValid(x, y, z), "Corner must be valid");
                grid.setOccupied(x, y, z);
                require(grid.isOccupied(x, y, z), "Corner must be accessible");
            }
        }
    }
    const std::array<std::array<int, 3>, 8> invalid{{
        {-1, 0, 0}, {0, -1, 0}, {0, 0, -1},
        {3, 0, 0}, {0, 4, 0}, {0, 0, 2},
        {std::numeric_limits<int>::min(), 0, 0},
        {0, 0, std::numeric_limits<int>::max()}
    }};
    for (const auto& p : invalid) {
        require(!grid.isValid(p[0], p[1], p[2]), "Invalid coordinate accepted");
        expectThrows<std::out_of_range>([&] { grid.isOccupied(p[0], p[1], p[2]); });
        expectThrows<std::out_of_range>([&] { grid.setOccupied(p[0], p[1], p[2]); });
    }
    VoxelGrid single(1, 1, 1, 0.25);
    single.setOccupied(0, 0, 0);
    require(single.isOccupied(0, 0, 0), "Single voxel must be accessible");
}

void testConstructorValidation() {
    for (int invalid : {0, -1, std::numeric_limits<int>::min()}) {
        expectThrows<std::invalid_argument>([&] { VoxelGrid grid(invalid, 2, 3, 1.0); });
        expectThrows<std::invalid_argument>([&] { VoxelGrid grid(2, invalid, 3, 1.0); });
        expectThrows<std::invalid_argument>([&] { VoxelGrid grid(2, 3, invalid, 1.0); });
    }
    for (double invalid : {0.0, -0.5, std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}) {
        expectThrows<std::invalid_argument>([&] { VoxelGrid grid(2, 3, 4, invalid); });
    }
    expectThrows<std::length_error>([] {
        const int largest = std::numeric_limits<int>::max();
        VoxelGrid grid(largest, largest, largest, 1.0);
    });
}

void testIndexing() {
    // Asymmetric dimensions expose incorrect strides and row/layer aliasing.
    // Set one voxel at a time and verify every other coordinate stays free.
    VoxelGrid grid(3, 4, 2, 0.5);
    for (int selected = 0; selected < 24; ++selected) {
        const int sx = selected % 3;
        const int sy = (selected / 3) % 4;
        const int sz = selected / 12;
        grid.setOccupied(sx, sy, sz);
        for (int z = 0; z < 2; ++z) {
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 3; ++x) {
                    require(grid.isOccupied(x, y, z) == (x + 3 * (y + 4 * z) == selected),
                            "3D coordinates must address distinct voxels");
                }
            }
        }
        grid.setOccupied(sx, sy, sz, false);
    }
}
} // namespace

int main() {
    try {
        testInitializationAndUpdates();
        testCoordinates();
        testConstructorValidation();
        testIndexing();
        std::cout << "All VoxelGrid tests passed\n";
    } catch (const std::exception& error) {
        std::cerr << "VoxelGrid test failure: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
