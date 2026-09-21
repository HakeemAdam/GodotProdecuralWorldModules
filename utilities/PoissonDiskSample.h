#pragma once


#include "GridOps.h"
#include "core/math/random_number_generator.h"
#include "core/math/vector2.h"
#include "core/variant/variant.h"
//#include <cstddef>

struct PoissonInput{
	Bounds2D bounds;
	float minDist;
	int maxAttempts;
};

struct PoissonOutput{
	PackedVector2Array points;
};


inline void GeneratePoissonSampling(const PoissonInput& input, PoissonOutput& output){
	float cellSize = input.minDist/ sqrt(2.0f);

	// Create grid
	SparseGrid2D grid = createSparseGrid(input.bounds, cellSize);
	PackedVector2Array points;
	PackedInt32Array active;

	// Add first point randomly and make it the active point
	Ref<RandomNumberGenerator> rng;
	rng.instantiate();

	Vector2 firstPoint = {
		rng->randf_range(grid.info.bounds.minX, grid.info.bounds.maxX),
		rng->randf_range(grid.info.bounds.minY, grid.info.bounds.maxY)
	};

	points.push_back(firstPoint);
	active.push_back(0);
	insertPointToSparseGrid(grid, firstPoint.x, firstPoint.y, 0);

	while(!active.is_empty()){
		// While active poinst are not empty, pick a random point and make it active
		int activeIdx = floor(rng->randf_range(0, active.size()));
		int pointIdx = active[activeIdx];
		Vector2 pos = points[pointIdx];
		bool found = false;

		// Pick a random point in the radius of the active point
		for(size_t iter =0; iter < input.maxAttempts; iter++){
			float angle = rng->randf_range(0, 2*3.14256 );
			float dist = rng->randf_range(input.minDist, input.minDist*2);

			// Pick a candidate position for the point
			Vector2 candidate;
			candidate.x = pos.x - cos(angle) * dist;
			candidate.y = pos.y - sin(angle) * dist;

			// Make sure the candidate is within the bounds
			if(candidate.x < input.bounds.minX || candidate. x >= input.bounds.maxX || candidate.y < input.bounds.minY || candidate.y >= input.bounds.maxY) {continue;}

			// Get the neighbor cells around the candidate
			bool valid = true;
			PackedInt32Array neighbours = querySparseGridNeighborHood(grid, candidate.x, candidate.y);

			for(int cellIdx=0; cellIdx < neighbours.size(); cellIdx ++ ){
				// Ensure candidate is withing the radius
				if (neighbours[cellIdx] != -1){
					Vector2 neighbor = points[neighbours[cellIdx]];
					float d = candidate.distance_squared_to(neighbor);

					if(d < input.minDist * input.minDist){
						valid = false;
					}
				}
			}

			// if yes, add the candidate to the list and repeat
			if (valid){
				int newIdx = points.size();
				points.push_back(candidate);
				insertPointToSparseGrid(grid, candidate.x, candidate.y, newIdx);
				active.push_back(newIdx);
				found=true;
				break;
			}

		}

		// if not found, select and active point
		if (!found){
			int last = active[active.size() - 1];
			active.set(activeIdx, last);
			active.resize(active.size() - 1);
		}
	}

	// Output found points
	for(const Vector2& point: points){
		output.points.push_back(point);
	}


};
