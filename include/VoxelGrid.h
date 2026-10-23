#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// Bounded occupancy grid: integer coordinates, with resolution in meters.
// Construction takes O(width * height * depth) time and space; access is O(1).
class VoxelGrid {
public:
    // Throws invalid_argument for invalid dimensions/resolution, length_error
    // for unsupported sizes, or bad_alloc if available memory is insufficient.
    VoxelGrid(int width, int height, int depth, double resolution);

    bool isValid(int x, int y, int z) const;
    // Both occupancy operations throw out_of_range for invalid coordinates.
    bool isOccupied(int x, int y, int z) const;
    void setOccupied(int x, int y, int z, bool occupied = true);

private:
    std::size_t index(int x, int y, int z) const;

    int width_;
    int height_;
    int depth_;
    double resolution_;
    std::vector<std::uint8_t> occupancy_;
};
