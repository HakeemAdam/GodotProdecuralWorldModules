#pragma once

#include "core/math/vector2.h"
#include <cmath>
#include <vector>

// bounds struct
struct Bounds2D {
    float minX, minY, maxX, maxY;
};

// grid struct
struct GridInfo2D {
    int numCols, numRows;
    float cellSize;
    Bounds2D bounds;
};

// create a grid and init data
inline GridInfo2D createGrid(Bounds2D bounds, float cellSize) {
    GridInfo2D grid{};
    grid.cellSize=cellSize;
    grid.bounds = bounds;
    grid.numCols = std::ceil((bounds.maxX-bounds.minX)/cellSize);
    grid.numRows =std::ceil((bounds.maxY-bounds.minY)/cellSize);
    return grid;
}

// convert col, row to flat index
inline int toFlat2D(int col, int row, int numCols) {
    return col + row * numCols;
}

// posToCell - convert pos x, y  to col, row
inline void posToCell(float posX, float posY, const GridInfo2D& grid, int& outCol, int& outRow) {
    outCol = std::floor((posX - grid.bounds.minX) / grid.cellSize);
    outRow = std::floor((posY - grid.bounds.minY) / grid.cellSize);
}

// posToFlat, convert x,y pos to flat idx
inline int posToFlat(float posX, float posY, const GridInfo2D& grid) {
    int col, row;
    posToCell(posX,posY,grid,col, row);
    return toFlat2D(col,row,grid.numCols);
}

// fromFat2d get the col and row based on flat idex and numcolss
inline void fromFlat2D(const int flat, const int numCols, int& outCol, int& outRow) {
    outCol = flat % numCols;
    outRow = flat / numCols;
}

// cell to pos, Retrun vector2 xy pos based on col, row and idx and grid
inline Vector2 cellToPos(int flat, const GridInfo2D& grid) {
    int col, row;
    fromFlat2D(flat, grid.numCols, col, row);
    float x = grid.bounds.minX + col * grid.cellSize + grid.cellSize * 0.5f;
    float y = grid.bounds.minY + row * grid.cellSize + grid.cellSize * 0.5f;
    return Vector2(x,y);
}

// Sparse grid

// struct - cell = indices, initialized to -1, meaning  not occupied, when occupied, holds the index [0,1,2,3,...]
struct SparseGrid2D {
    std::vector<int> cells; // the list of indices/cells
    GridInfo2D info;        // the grid, storing info
};

// create sparse grid
inline SparseGrid2D createSparseGrid(Bounds2D bounds, float cellSize) {
    SparseGrid2D grid ={};
    grid.info = createGrid(bounds,cellSize); // initialize grid
    int gridSize = grid.info.numCols * grid.info.numRows;
    grid.cells.resize(gridSize, -1);  //resize cel to match the spaces in the grid, and init to -1, not occupied
    return grid;
}

// insert a point - x,y pos and index
inline bool insertPointToSparseGrid(SparseGrid2D& grid,float posX, float posY, int pointIndex) {
    int idx = posToFlat(posX,posY,grid.info);
    if (grid.cells[idx] != -1) return false; // not empty
    grid.cells[idx] = pointIndex; // pass the index to the list of cell
    return true;

}
// query and cell / based on x, y pos. same as checking occupancy

inline int querySparseGrid(const SparseGrid2D& grid, float posX, float posY) {
    int idx = posToFlat(posX,posY,grid.info);
    return grid.cells[idx];

}

//clear grid, set all element to -1

inline void clearSparseGrid(SparseGrid2D& grid) {
    for ( int& cell: grid.cells) cell=-1;
}

// get 8X8 neighbours, based on col and row
inline std::vector<int> queryGridNeighbors(int col, int row, int numCols, int numRows) {
    std::vector<int> neighbourCells;
    for ( int dy = -1; dy <= 1; dy ++) {
        for ( int dx = -1; dx <= 1; dx++) {
            int nx = dx +col;
            int ny = dy + row;

            if ( nx < 0 || nx >= numCols) continue;
            if ( ny < 0 || ny >= numRows) continue;

            int idx = nx + ny * numCols;
            neighbourCells.push_back(idx);
        }
    }
    return neighbourCells;
}


// querySpareseGridNeighboorhood, given and input x,y position, retrun all the neigbors
inline std::vector<int> querySparseGridNeighborHood(const SparseGrid2D& grid, float posX, float posY) {
    int col, row;
    posToCell(posX,posY, grid.info, col, row);

    std::vector<int> Neighbourhood = queryGridNeighbors(col, row, grid.info.numCols, grid.info.numRows);
    std::vector<int> result;
    for (int cellIdx: Neighbourhood) {
        result.push_back(grid.cells[cellIdx]);
    }
    return result;
}

// DenseGrid

struct DenseGrid2D {
    std::vector<std::vector<int>> cells;
    GridInfo2D info;
};

inline DenseGrid2D createDenseGrid(Bounds2D bounds, float cellSize) {
    DenseGrid2D grid={};
    grid.info=createGrid(bounds, cellSize);
    int gridSize =grid.info.numCols*grid.info.numRows;
    grid.cells.resize(gridSize);
    return grid;
}

inline void insertPointToDenseGrid(DenseGrid2D& grid,float posX, float posY, int pointIndex) {
    int idx = posToFlat(posX,posY,grid.info);
    grid.cells[idx].push_back(pointIndex);
}

inline std::vector<int> queryDenseGrid(const DenseGrid2D& grid, float posX, float posY) {
    int idx = posToFlat(posX, posY, grid.info);
    return grid.cells[idx];
}

inline void clearDenseGrid(DenseGrid2D& grid) {
    for (std::vector<int>& cell: grid.cells) {
        cell.clear();
    }
}

inline std::vector<int> queryDenseGridNeighborHood(const DenseGrid2D& grid, float posX, float posY) {
    int col, row;
    posToCell(posX,posY,grid.info,col, row);
    std::vector<int> Neighbourhood =queryGridNeighbors(col, row, grid.info.numCols, grid.info.numRows);

    std::vector<int> result;
    for ( int cellIdx: Neighbourhood) {
        for ( int pointIdx: grid.cells[cellIdx]) {
            result.push_back(pointIdx);
        }
    }
    return result;
}

inline std::vector<int> queryDenseGridNeighborHoodRadius(const DenseGrid2D& grid, const std::vector<Vector2>& points, float queryX, float queryY, float radius) {
    // find all points near target positions
    std::vector<int> candidates = queryDenseGridNeighborHood(grid, queryX, queryY);

    std::vector<int> result;
    // loop through candidate indices
    for ( int pointIdx: candidates ) {
        // check the distance between the target, input, and all point in the candidates array by index
	Vector2 targetPoint = points[pointIdx];
        float dist = targetPoint.distance_squared_to(Vector2(queryX,queryY));
        // push within radius
        if (dist <= radius) {
            result.push_back(pointIdx);
        }
    }
    return result;
}
