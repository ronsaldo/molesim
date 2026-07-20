#ifndef MOLESIM_GRID_HPP
#define MOLESIM_GRID_HPP

#include "AABox.hpp"
#include "IVector3.hpp"
#include <assert.h>
#include <stdio.h>

namespace Molesim
{

/**
 * A grid spatial subdivision data structure. Based on the Acceleration Structure implemented by UDock2
 */
template<typename PT>
class Grid
{
public:
    typedef PT PayloadType;

    void setupForBoundingBox(const AABox &boundingBox)
    {
        origin = boundingBox.minCorner;
        extent = boundingBox.extent();
        cellSize = extent / Vector3(gridSize.x, gridSize.y, gridSize.z);
        grid.resize(gridSize.x * gridSize.y * gridSize.z);
    }

    void addPoint(const Vector3 &point, const PayloadType &payload)
    {
        auto gridPosition = getGridPosition(point);
        auto gridIndex = getGridIndex(gridPosition);
        assert(gridIndex < grid.size());
        grid[gridIndex].push_back(payload);
    }

    // Based on particle simulation samples
    // Ref: https://github.com/zchee/cuda-sample/blob/master/5_Simulations/particles/particles_kernel_impl.cuh#L109
    IVector3 getGridPosition(const Vector3 &position) const
    {
        IVector3 gridPos;
        gridPos.x = int(floor((position.x - origin.x) /cellSize.x ));
        gridPos.y = int(floor((position.y - origin.y) /cellSize.y ));
        gridPos.z = int(floor((position.z - origin.z) /cellSize.z ));
        return gridPos;
    }

    size_t getGridIndex(IVector3 gridPosition) const
    {
        // Wrap grid. Assumes size is a power of two.
        gridPosition.x &= gridSize.x - 1;
        gridPosition.y &= gridSize.y - 1;
        gridPosition.z &= gridSize.z - 1;
        return
            gridPosition.z*gridSize.y*gridSize.x +
            gridPosition.y*gridSize.x +
            gridPosition.x;
    }

    template<typename FT>
    void nodesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        auto minGridPosition = max(getGridPosition(box.minCorner), IVector3(0));
        auto maxGridPosition = min(getGridPosition(box.maxCorner), gridSize - IVector3(1));
        //printf("boxMin: %f %f %f\n", box.minCorner.x, box.minCorner.y, box.minCorner.z);
        //printf("boxMax: %f %f %f\n", box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);

        //printf("minGrid: %d %d %d\n", minGridPosition.x, minGridPosition.y, minGridPosition.z);
        //printf("maxGrid: %d %d %d\n", maxGridPosition.x, maxGridPosition.y, maxGridPosition.z);
        for(int x = minGridPosition.x; x <= maxGridPosition.x; ++x)
        {
            for(int y = minGridPosition.y; y <= maxGridPosition.y; ++y)
            {
                for(int z = minGridPosition.z; z <= maxGridPosition.z; ++z)
                {
                    auto currentCell = IVector3(x, y, z) ;
                    auto currentCellIndex = getGridIndex(currentCell);

                    auto &cell = grid[currentCellIndex];
                    for(auto cellElement : cell)
                        aBlock(cellElement);
                }
            }
        }
    }

    Vector3 origin;
    Vector3 extent;
    Vector3 cellSize;
    IVector3 gridSize = IVector3(16, 16, 16);
    std::vector<std::vector<PayloadType>> grid;
};

} // End of namespace Molesim

#endif //MOLESIM_GRID_HPP