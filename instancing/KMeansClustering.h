#pragma once
#include "core/math/random_number_generator.h"
#include "core/math/vector2.h"
#include "core/object/ref_counted.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include <cfloat>


struct Point{
	Vector2 pos;
	int32_t clusterId = -1;
};

struct KMeansInput{
	int maxIterations;
	int k;
	Vector<Point> points;
};

struct KMeansOutput{
	PackedVector2Array outputPoints;
	PackedInt32Array pointClusters;
	PackedInt32Array clusterIds;
};

inline void GenerateKMeansCluster(KMeansInput& input, KMeansOutput& output){

	int numPoints = input.points.size();
	int k_cluster = input.k;

	if(numPoints ==0 || k_cluster <=0) {return;}

	Vector<Vector2> centroids;
	centroids.resize(k_cluster);

	Ref<RandomNumberGenerator> rng;
	rng.instantiate();


	// pick random centers
	for(int i =0; i < k_cluster; i++){
		int randIdx = rng->randi_range(0, numPoints-1);

		Vector2 newPos = Vector2(input.points[randIdx].pos.x, input.points[randIdx].pos.y);
		centroids.set(i, newPos);
	}

	for(int iter = 0; iter < input.maxIterations; iter++){

		bool clusterChanges = false;

		// find closetes centorid for each point
		//
		for(int i =0; i < numPoints; i++){
			float minDist = FLT_MAX;
			int closestCluster = -1;

			for(int clusterId =0; clusterId < k_cluster; clusterId++){
				float dist = input.points[i].pos.distance_squared_to(centroids[clusterId]);

				if(dist < minDist){
					minDist = dist;
					closestCluster = clusterId;
				}
			}

			// assign points to closest cluster
			if(input.points[i].clusterId != closestCluster){
				auto p = input.points[i];
				p.clusterId = closestCluster;
				input.points.set(i, p);
				clusterChanges = true;
			}
		}

		// check for convergence
		if(!clusterChanges){
			break;
		}

		// sum points in clusters
		Vector<Vector2> centroidSum;
		Vector<int> clusterCount;
		centroidSum.resize(k_cluster);
		clusterCount.resize(k_cluster);

		for(int c =0; c < k_cluster; c++){
			centroidSum.set(c, Vector2(0,0));
			clusterCount.set(c, 0);
		}

		// calculate mean of clustes
		for(int i =0; i < numPoints; i++){
			int clusterId = input.points[i].clusterId;
			if(clusterId >=0 && clusterId < k_cluster){
				Vector2 currentSum = centroidSum[clusterId] + input.points[i].pos;
				centroidSum.set(clusterId, currentSum);
				int currentCount = clusterCount[clusterId] + 1;
				clusterCount.set(clusterId, currentCount);
			}
		}


		// calculate new mean position of centroids
		for (int j = 0; j < k_cluster; j++){
			if(clusterCount[j] > 0){
				Vector2 meanPos = centroidSum[j] / clusterCount[j];
				centroids.set(j, meanPos);
			}
		}
	}

	// Output
	output.outputPoints.clear();
	output.clusterIds.clear();
	output.pointClusters.clear();

	output.outputPoints.resize(numPoints);
	output.pointClusters.resize(numPoints);

	for(int i = 0; i < numPoints; i++ ){
		output.outputPoints.set(i, input.points[i].pos);
		output.pointClusters.set(i, input.points[i].clusterId);
	}

	output.clusterIds.resize(k_cluster);
	for(int i =0; i < k_cluster; i++){
		output.clusterIds.set(i, i);
	}

};


