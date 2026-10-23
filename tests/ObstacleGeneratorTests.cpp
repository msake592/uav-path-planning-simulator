#include "ObstacleGenerator.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

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

std::vector<bool> snapshot(const VoxelGrid& grid) {
    std::vector<bool> result;
    for (int z = 0; z < grid.depth(); ++z) {
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                result.push_back(grid.isOccupied(x, y, z));
            }
        }
    }
    return result;
}

void testBoxes() {
    VoxelGrid grid(5, 4, 3, 0.5);
    ObstacleGenerator::placeBox(grid, {2, 1, 1}, 3, 3, 2);
    ObstacleGenerator::placeBox(grid, {4, 3, 2}, 1, 1, 1); // Overlap at last voxel.
    ObstacleGenerator::placeBox(grid, {0, 0, 0}, 1, 1, 1);
    for (int z = 0; z < 3; ++z) {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 5; ++x) {
                const bool expected = (x >= 2 && y >= 1 && z >= 1)
                    || (x == 0 && y == 0 && z == 0);
                require(grid.isOccupied(x, y, z) == expected, "Incorrect box extent");
            }
        }
    }
    const auto before = snapshot(grid);
    for (int invalid : {0, -1, std::numeric_limits<int>::min()}) {
        expectThrows<std::invalid_argument>([&] {
            ObstacleGenerator::placeBox(grid, {0, 0, 0}, invalid, 1, 1);
        });
        expectThrows<std::invalid_argument>([&] {
            ObstacleGenerator::placeBox(grid, {0, 0, 0}, 1, invalid, 1);
        });
        expectThrows<std::invalid_argument>([&] {
            ObstacleGenerator::placeBox(grid, {0, 0, 0}, 1, 1, invalid);
        });
    }
    const std::array<VoxelCoordinate, 8> invalidStarts{{
        {-1, 0, 0}, {0, -1, 0}, {0, 0, -1},
        {5, 0, 0}, {0, 4, 0}, {0, 0, 3},
        {std::numeric_limits<int>::max(), 0, 0},
        {0, std::numeric_limits<int>::min(), 0}
    }};
    for (auto start : invalidStarts) {
        expectThrows<std::out_of_range>([&] {
            ObstacleGenerator::placeBox(grid, start, 1, 1, 1);
        });
    }
    for (auto dimensions : {VoxelCoordinate{2, 1, 1}, VoxelCoordinate{1, 2, 1},
                            VoxelCoordinate{1, 1, 2},
                            VoxelCoordinate{std::numeric_limits<int>::max(), 1, 1}}) {
        expectThrows<std::out_of_range>([&] {
            ObstacleGenerator::placeBox(grid, {4, 3, 2},
                                       dimensions.x, dimensions.y, dimensions.z);
        });
    }
    require(snapshot(grid) == before, "Invalid boxes must not mutate grid");
    ObstacleGenerator::placeBox(grid, {0, 0, 0}, 5, 4, 3);
    for (bool occupied : snapshot(grid)) {
        require(occupied, "Full-grid box must occupy every voxel");
    }
}

void testRandomExtremesAndEndpoints() {
    VoxelGrid grid(5, 4, 3, 1.0);
    ObstacleGenerator::generateRandom(grid, 0.0, 7);
    for (bool occupied : snapshot(grid)) {
        require(!occupied, "p=0 must leave an empty grid empty");
    }
    grid.setOccupied(2, 2, 1);
    grid.setOccupied(0, 0, 0);
    grid.setOccupied(4, 3, 2);
    const VoxelCoordinate start{0, 0, 0}, goal{4, 3, 2};
    ObstacleGenerator::generateRandom(grid, 0.0, 7, start, goal);
    require(grid.isOccupied(2, 2, 1), "p=0 must preserve existing obstacles");
    require(!grid.isOccupied(0, 0, 0) && !grid.isOccupied(4, 3, 2),
            "Endpoints must be cleared even at p=0");
    ObstacleGenerator::generateRandom(grid, 1.0, 7, start, goal);
    const auto pattern = snapshot(grid);
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        require(pattern[i] == (i != 0 && i != pattern.size() - 1),
                "p=1 must occupy all eligible voxels");
    }
    ObstacleGenerator::generateRandom(grid, 1.0, 7);
    for (bool occupied : snapshot(grid)) {
        require(occupied, "Without endpoints p=1 must occupy entire grid");
    }
    VoxelGrid single(1, 1, 1, 1.0);
    ObstacleGenerator::generateRandom(single, 1.0, 7, start, start);
    require(!single.isOccupied(0, 0, 0), "Coincident endpoints must remain free");
}

void testRandomValidation() {
    VoxelGrid grid(5, 4, 3, 1.0);
    grid.setOccupied(0, 0, 0);
    const auto before = snapshot(grid);
    for (double invalid : {-0.1, 1.1, std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}) {
        expectThrows<std::invalid_argument>([&] {
            ObstacleGenerator::generateRandom(grid, invalid, 7);
        });
    }
    for (auto invalid : {VoxelCoordinate{-1, 0, 0}, VoxelCoordinate{0, -1, 0},
                         VoxelCoordinate{0, 0, -1}, VoxelCoordinate{5, 0, 0},
                         VoxelCoordinate{0, 4, 0}, VoxelCoordinate{0, 0, 3}}) {
        expectThrows<std::out_of_range>([&] {
            ObstacleGenerator::generateRandom(grid, 1.0, 7, invalid);
        });
        expectThrows<std::out_of_range>([&] {
            ObstacleGenerator::generateRandom(grid, 1.0, 7,
                                             VoxelCoordinate{0, 0, 0}, invalid);
        });
    }
    require(snapshot(grid) == before, "Invalid random inputs must not mutate grid");
}

void testSeeds() {
    VoxelGrid first(8, 7, 6, 1.0), second(8, 7, 6, 1.0);
    ObstacleGenerator::placeBox(first, {1, 1, 1}, 2, 2, 2);
    ObstacleGenerator::placeBox(second, {1, 1, 1}, 2, 2, 2);
    ObstacleGenerator::generateRandom(first, 0.35, 42);
    ObstacleGenerator::generateRandom(second, 0.35, 42);
    const auto baseline = snapshot(first);
    require(baseline == snapshot(second), "Same seed must reproduce pattern");
    require(first.isOccupied(1, 1, 1), "Random generation must preserve boxes");

    // Fixed seed corpus: require some variation, not pairwise uniqueness.
    bool variation = false;
    for (std::uint32_t seed = 0; seed < 16; ++seed) {
        VoxelGrid candidate(8, 7, 6, 1.0);
        ObstacleGenerator::placeBox(candidate, {1, 1, 1}, 2, 2, 2);
        ObstacleGenerator::generateRandom(candidate, 0.35, seed);
        variation = variation || snapshot(candidate) != baseline;
    }
    require(variation, "Fixed seed corpus should demonstrate different environments");
}
} // namespace

int main() {
    try {
        testBoxes();
        testRandomExtremesAndEndpoints();
        testRandomValidation();
        testSeeds();
        std::cout << "All ObstacleGenerator tests passed\n";
    } catch (const std::exception& error) {
        std::cerr << "ObstacleGenerator test failure: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
