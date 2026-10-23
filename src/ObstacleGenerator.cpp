#include "ObstacleGenerator.h"

#include <cmath>
#include <random>
#include <stdexcept>

void ObstacleGenerator::placeBox(VoxelGrid& grid, VoxelCoordinate start,
                                 int width, int height, int depth) {
    if (width <= 0 || height <= 0 || depth <= 0) {
        throw std::invalid_argument("Box dimensions must be positive");
    }
    // Subtraction after validating start avoids overflowing start + dimension.
    if (!grid.isValid(start.x, start.y, start.z)
        || width > grid.width() - start.x
        || height > grid.height() - start.y
        || depth > grid.depth() - start.z) {
        throw std::out_of_range("Box must fit inside the grid");
    }
    for (int z = start.z; z < start.z + depth; ++z) {
        for (int y = start.y; y < start.y + height; ++y) {
            for (int x = start.x; x < start.x + width; ++x) {
                grid.setOccupied(x, y, z);
            }
        }
    }
}

void ObstacleGenerator::generateRandom(VoxelGrid& grid, double probability,
                                       std::uint32_t seed,
                                       std::optional<VoxelCoordinate> start,
                                       std::optional<VoxelCoordinate> goal) {
    if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("Probability must be finite and in [0,1]");
    }
    for (const auto& endpoint : {start, goal}) {
        if (endpoint && !grid.isValid(endpoint->x, endpoint->y, endpoint->z)) {
            throw std::out_of_range("Endpoint must be inside the grid");
        }
    }
    std::mt19937 engine(seed);
    for (int z = 0; z < grid.depth(); ++z) {
        for (int y = 0; y < grid.height(); ++y) {
            for (int x = 0; x < grid.width(); ++x) {
                const VoxelCoordinate coordinate{x, y, z};
                const double sample = static_cast<double>(engine()) / 4294967296.0;
                if ((start && coordinate == *start) || (goal && coordinate == *goal)) {
                    grid.setOccupied(x, y, z, false);
                } else if (sample < probability) {
                    grid.setOccupied(x, y, z);
                }
            }
        }
    }
}
