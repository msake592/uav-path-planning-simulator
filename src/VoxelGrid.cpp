#include "VoxelGrid.h"

#include <cmath>
#include <stdexcept>

VoxelGrid::VoxelGrid(int width, int height, int depth, double resolution)
    : width_(width), height_(height), depth_(depth), resolution_(resolution) {
    if (width <= 0 || height <= 0 || depth <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    if (!std::isfinite(resolution) || resolution <= 0.0) {
        throw std::invalid_argument("Resolution must be finite and positive");
    }

    // Guard each multiplication before computing it. max_size() also limits
    // the count to what the vector's allocator and difference type support.
    std::size_t count = 1;
    for (int dimension : {width, height, depth}) {
        const auto size = static_cast<std::size_t>(dimension);
        if (size > occupancy_.max_size() / count) {
            throw std::length_error("Grid dimensions exceed storage limits");
        }
        count *= size;
    }
    occupancy_.assign(count, 0);
}

bool VoxelGrid::isValid(int x, int y, int z) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_
        && z >= 0 && z < depth_;
}

std::size_t VoxelGrid::index(int x, int y, int z) const {
    if (!isValid(x, y, z)) {
        throw std::out_of_range("Voxel coordinates are outside the grid");
    }
    // X varies fastest, then Y, then Z. Convert before any multiplication.
    return static_cast<std::size_t>(x) + static_cast<std::size_t>(width_)
        * (static_cast<std::size_t>(y) + static_cast<std::size_t>(height_)
        * static_cast<std::size_t>(z));
}

bool VoxelGrid::isOccupied(int x, int y, int z) const {
    return occupancy_[index(x, y, z)] != 0;
}

void VoxelGrid::setOccupied(int x, int y, int z, bool occupied) {
    occupancy_[index(x, y, z)] = occupied ? 1 : 0;
}
