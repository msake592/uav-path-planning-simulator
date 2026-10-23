#pragma once

#include "VoxelGrid.h"

#include <cstdint>
#include <optional>

struct VoxelCoordinate {
    int x;
    int y;
    int z;
    bool operator==(const VoxelCoordinate&) const = default;
};

class ObstacleGenerator {
public:
    // Occupies [start, start + dimensions) on each axis. Overlap is allowed.
    // Invalid dimensions throw invalid_argument; invalid bounds throw
    // out_of_range. Validation completes before any voxel is changed.
    // Time O(width * height * depth), extra space O(1).
    static void placeBox(VoxelGrid& grid, VoxelCoordinate start,
                         int width, int height, int depth);

    // Adds independent per-voxel obstacles; existing occupancy is preserved
    // except designated endpoints, which are explicitly cleared (even at p=0).
    // Probability must be finite and in [0,1], otherwise invalid_argument.
    // Invalid endpoints throw out_of_range before any changes.
    // Same initial grid, parameters and seed give identical results.
    // Samples mt19937 in Z/Y/X order using engine()/2^32 < probability.
    // Probability has 32-bit sampling precision; no path connectivity guarantee.
    // Time O(grid volume), extra space O(1).
    static void generateRandom(VoxelGrid& grid, double probability,
                               std::uint32_t seed,
                               std::optional<VoxelCoordinate> start = std::nullopt,
                               std::optional<VoxelCoordinate> goal = std::nullopt);
};
