#pragma once

#include "core/math/vector2.h"
#include "core/variant/variant.h"

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
    grid.numCols = ceil((bounds.maxX-bounds.minX)/cellSize);
    grid.numRows = ceil((bounds.maxY-bounds.minY)/cellSize);
    return grid;
}

// convert col, row to flat index
inline int toFlat2D(int col, int row, int numCols) {
    return col + row * numCols;
}

// posToCell - convert pos x, y  to col, row
inline void posToCell(float posX, float posY, const GridInfo2D& grid, int& outCol, int& outRow) {
    outCol = floor((posX - grid.bounds.minX) / grid.cellSize);
    outRow = floor((posY - grid.bounds.minY) / grid.cellSize);
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
    PackedInt32Array cells; // the list of indices/cells
    GridInfo2D info;        // the grid, storing info
};

// create sparse grid
inline SparseGrid2D createSparseGrid(Bounds2D bounds, float cellSize) {
    SparseGrid2D grid ={};
    grid.info = createGrid(bounds,cellSize); // initialize grid
    int gridSize = grid.info.numCols * grid.info.numRows;
    grid.cells.resize(gridSize);//resize cel to match the spaces in the grid, and init to -1, not occupied
    grid.cells.fill(-1);
	return grid;
}

// insert a point - x,y pos and index
inline bool insertPointToSparseGrid(SparseGrid2D& grid,float posX, float posY, int pointIndex) {
    int idx = posToFlat(posX,posY,grid.info);
    if (grid.cells[idx] != -1){
		return false; // not empty
		}
    grid.cells.set(idx, pointIndex); // pass the index to the list of cell
    return true;

}
// query and cell / based on x, y pos. same as checking occupancy

inline int querySparseGrid(const SparseGrid2D& grid, float posX, float posY) {
    int idx = posToFlat(posX,posY,grid.info);
    return grid.cells[idx];

}

//clear grid, set all element to -1

inline void clearSparseGrid(SparseGrid2D& grid) {
    for ( int& cell: grid.cells) {cell=-1;}
}

// get 8X8 neighbours, based on col and row
inline PackedInt32Array queryGridNeighbors(int col, int row, int numCols, int numRows) {
    PackedInt32Array neighbourCells;
    for ( int dy = -1; dy <= 1; dy ++) {
        for ( int dx = -1; dx <= 1; dx++) {
            int nx = dx +col;
            int ny = dy + row;

            if ( nx < 0 || nx >= numCols) {continue;}
            if ( ny < 0 || ny >= numRows) {continue;}

            int idx = nx + ny * numCols;
            neighbourCells.push_back(idx);
        }
    }
    return neighbourCells;
}


// querySpareseGridNeighboorhood, given and input x,y position, retrun all the neigbors
inline PackedInt32Array querySparseGridNeighborHood(const SparseGrid2D& grid, float posX, float posY) {
    int col, row;
    posToCell(posX,posY, grid.info, col, row);

    PackedInt32Array Neighbourhood = queryGridNeighbors(col, row, grid.info.numCols, grid.info.numRows);
    PackedInt32Array result;
    for (int cellIdx: Neighbourhood) {
        result.push_back(grid.cells[cellIdx]);
    }
    return result;
}

// DenseGrid

struct DenseGrid2D {
    Vector<PackedInt32Array>cells;
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

	PackedInt32Array cell_array = grid.cells[idx];
	cell_array.push_back(pointIndex);
	grid.cells.set(idx, cell_array);

    //grid.cells[idx].push_back(pointIndex);
}

inline PackedInt32Array queryDenseGrid(const DenseGrid2D& grid, float posX, float posY) {
    int idx = posToFlat(posX, posY, grid.info);
    return grid.cells[idx];
}

inline void clearDenseGrid(DenseGrid2D& grid) {
    for (PackedInt32Array& cell: grid.cells) {
        cell.clear();
    }
}

inline PackedInt32Array queryDenseGridNeighborHood(const DenseGrid2D& grid, float posX, float posY) {
    int col, row;
    posToCell(posX,posY,grid.info,col, row);
    PackedInt32Array Neighbourhood =queryGridNeighbors(col, row, grid.info.numCols, grid.info.numRows);

    PackedInt32Array result;
    for ( int cellIdx: Neighbourhood) {
        for ( int pointIdx: grid.cells[cellIdx]) {
            result.push_back(pointIdx);
        }
    }
    return result;
}

inline PackedInt32Array queryDenseGridNeighborHoodRadius(const DenseGrid2D& grid, const PackedVector2Array& points, float queryX, float queryY, float radius) {
    // find all points near target positions
    PackedInt32Array candidates = queryDenseGridNeighborHood(grid, queryX, queryY);

    PackedInt32Array result;
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
