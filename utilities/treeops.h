#ifndef TREEOPS_H
#define TREEOPS_H

#include <algorithm>
#include <cfloat>
#include <cstdlib>
#include <vector>
#include "core/math/vector3.h"
#include "core/math/vector2.h"
//KD Tree

struct KDNode2D{
    Vector2 point;
    int axis;
    KDNode2D* left;
    KDNode2D* right;
};

struct KDNode3D{
	Vector3 point;
	int axis;
	KDNode3D* left;
	KDNode3D* right;
};

inline int getAxis(int depth, int numDims) {
    return  depth % numDims;
}

inline float getAxisValue(Vector2 point, int axis) {
    if (axis==0) {return point.x;}
    return point.y;
}

inline float getAxisValue(Vector3 point, int axis) {
    if (axis==0) {return point.x;}
    if (axis==1) {return point.y;}
    return point.z;
}

inline int findMedianIndex(std::vector<Vector2>& points, int axis) {
    std::sort(points.begin(), points.end(), [axis](Vector2 a, Vector2 b) {
        return getAxisValue(a, axis) < getAxisValue(b, axis);
    });
    return  points.size() /2;
}

inline int findMedianIndex(std::vector<Vector3>& points, int axis) {
    std::sort(points.begin(), points.end(), [axis](Vector3 a, Vector3 b) {
        return getAxisValue(a, axis) < getAxisValue(b, axis);
    });
    return  points.size() /2;
}


inline KDNode2D* buildKDTree(std::vector<Vector2>& points, int depth = 0) {
    if (points.empty()) {
        return nullptr;
    }

    int axis = depth % 2;
    int medianIdx = findMedianIndex(points, axis);

    KDNode2D* node = new KDNode2D();
    node->point = points[medianIdx];
    node->axis = axis;
    node->left =nullptr;
    node->right=nullptr;

    std::vector<Vector2> leftPoints(points.begin(), points.begin() + medianIdx);
    std::vector<Vector2> rightPoints(points.begin() + medianIdx + 1, points.end());

    node->left = buildKDTree(leftPoints, depth+1);
    node->right = buildKDTree(rightPoints, depth+1);

    return node;
}

inline float distanceToPlane(Vector2 query, KDNode2D* node){
	return std::abs(getAxisValue(query, node->axis) - getAxisValue(node->point, node->axis));
}

inline void nearestNeighbour(KDNode2D* node, Vector2 query, KDNode2D*& best, float& bestDist){
	if(node == nullptr) {return;}

	float d = query.distance_to(node->point);
	if ( d < bestDist){
		bestDist =d;
		best = node;
	}

	int axis = node->axis;
	float queryVal = getAxisValue(query, axis);
	float nodeVal = getAxisValue(node->point, axis);

	KDNode2D* first = (queryVal < nodeVal) ? node->left : node->right;
	KDNode2D* second = (queryVal < nodeVal) ? node->right : node->left;

	nearestNeighbour(first, query, best, bestDist);

	float distToPlane = std::abs(queryVal - nodeVal);
	if(distToPlane < bestDist){
		nearestNeighbour(second, query, best, bestDist);
	}
}

inline KDNode2D* findNearest(KDNode2D* root, Vector2 query){
	KDNode2D* best = nullptr;
	float bestDist = FLT_MAX;
	nearestNeighbour(root, query, best, bestDist);
	return best;
}

inline void radiusSearch(KDNode2D* node, Vector2 query, float radius, std::vector<Vector2>& results){
	if (node == nullptr) { return; }

	float d = query.distance_to(node->point);
	if (d <= radius){
		results.push_back(node->point);
	}

	float distToPlane = std::abs(getAxisValue(query, node->axis) - getAxisValue(node->point, node->axis));

	float queryVal = getAxisValue(query, node->axis);
	float nodeVal = getAxisValue(node->point, node->axis);

	if (queryVal < nodeVal){
		radiusSearch(node->left, query, radius, results);
		if(distToPlane <= radius){
			radiusSearch(node->right, query, radius, results);
		}
	} else {
		radiusSearch(node->right, query, radius, results);
		if(distToPlane <= radius){
			radiusSearch(node->left, query, radius, results);
		}
	}
}


inline void freeKDTree(KDNode2D* node){
	if (node == nullptr) {return;}
	freeKDTree(node->left);
	freeKDTree(node->right);
	delete node;
}

// 3D


inline KDNode3D* buildKDTree(std::vector<Vector3>& points, int depth = 0) {
    if (points.empty()) {
        return nullptr;
    }

    int axis = depth % 3;
    int medianIdx = findMedianIndex(points, axis);

    KDNode3D* node = new KDNode3D();
    node->point = points[medianIdx];
    node->axis = axis;
    node->left =nullptr;
    node->right=nullptr;

    std::vector<Vector3> leftPoints(points.begin(), points.begin() + medianIdx);
    std::vector<Vector3> rightPoints(points.begin() + medianIdx + 1, points.end());

    node->left = buildKDTree(leftPoints, depth+1);
    node->right = buildKDTree(rightPoints, depth+1);

    return node;
}

inline float distanceToPlane(Vector3 query, KDNode3D* node){
	return std::abs(getAxisValue(query, node->axis) - getAxisValue(node->point, node->axis));
}

inline void nearestNeighbour(KDNode3D* node, Vector3 query, KDNode3D*& best, float& bestDist){
	if(node == nullptr) {return;}

	float d = query.distance_to(node->point);
	if ( d < bestDist){
		bestDist =d;
		best = node;
	}

	int axis = node->axis;
	float queryVal = getAxisValue(query, axis);
	float nodeVal = getAxisValue(node->point, axis);

	KDNode3D* first = (queryVal < nodeVal) ? node->left : node->right;
	KDNode3D* second = (queryVal < nodeVal) ? node->right : node->left;

	nearestNeighbour(first, query, best, bestDist);

	float distToPlane = std::abs(queryVal - nodeVal);
	if(distToPlane < bestDist){
		nearestNeighbour(second, query, best, bestDist);
	}
}

inline KDNode3D* findNearest(KDNode3D* root, Vector3 query){
	KDNode3D* best = nullptr;
	float bestDist = FLT_MAX;
	nearestNeighbour(root, query, best, bestDist);
	return best;
}

inline void radiusSearch(KDNode3D* node, Vector3 query, float radius, std::vector<Vector3>& results){
	if (node == nullptr) { return; }

	float d = query.distance_to(node->point);
	if (d <= radius){
		results.push_back(node->point);
	}

	float distToPlane = std::abs(getAxisValue(query, node->axis) - getAxisValue(node->point, node->axis));

	float queryVal = getAxisValue(query, node->axis);
	float nodeVal = getAxisValue(node->point, node->axis);

	if (queryVal < nodeVal){
		radiusSearch(node->left, query, radius, results);
		if(distToPlane <= radius){
			radiusSearch(node->right, query, radius, results);
		}
	} else {
		radiusSearch(node->right, query, radius, results);
		if(distToPlane <= radius){
			radiusSearch(node->left, query, radius, results);
		}
	}
}


inline void freeKDTree(KDNode3D* node){
	if (node == nullptr) {return;}
	freeKDTree(node->left);
	freeKDTree(node->right);
	delete node;
}


// Quad Tree

struct AABB2{
	Vector2 center;
	float halfSize;
};

struct QuadNode{
	AABB2 bounds;
	std::vector<int> PointIndices;
	QuadNode* children[4] = {nullptr};
	int depth = 0;
	bool isLeaf() const {return children[0]== nullptr;}
};


inline int getQuadrant(Vector2 point, Vector2 center){
	int q = 0;
	if (point.x >= center.x) {q |= 1;}
	if(point.y >= center.y) {q |= 2;}
	return q;
}

inline Vector2 getChildCenters2D(Vector2 parentCenter, float parentHalfSize, int quadrant){
	float offset = parentHalfSize * 0.5f;
	Vector2 c = parentCenter;

	c.x += (quadrant & 1) ? offset : -offset;
	c.y += (quadrant & 2) ? offset : -offset;

	return c;
}


inline void subdivide(QuadNode* node){
	for (int i =0; i < 4; i++){
		QuadNode* child = new QuadNode();
		child->bounds.center = getChildCenters2D(node->bounds.center, node->bounds.halfSize, i);
		child->bounds.halfSize = node->bounds.halfSize * 0.5f;
		child->depth = node->depth + 1;
		node->children[i] = child;
	}
}


inline bool shouldSplit(int numPoints, int depth, int maxPointsPerLeaf, int maxDepth){
  	return numPoints > maxPointsPerLeaf && depth < maxDepth;
}


inline void insert(QuadNode* node, int pointIndex, const std::vector<Vector2>& positions, int maxPointsPerLeaf, int maxDepth){

	Vector2 pos = positions[pointIndex];

	if (node->isLeaf()) {
		node->PointIndices.push_back(pointIndex);

		if (shouldSplit((int)node->PointIndices.size(), node->depth,maxPointsPerLeaf, maxDepth)){
			subdivide(node);
			for(int idx : node->PointIndices){
				int quadrant = getQuadrant(positions[idx], node->bounds.center);
				insert(node->children[quadrant], idx, positions, maxPointsPerLeaf,maxDepth);
			}
			node->PointIndices.clear();
		}
	return;
	}
	int quadrant = getQuadrant(pos, node->bounds.center);
	insert(node->children[quadrant], pointIndex, positions, maxPointsPerLeaf, maxDepth);
}


inline QuadNode* buildQuadtree(const std::vector<Vector2>& positions, AABB2 rootBounds, int maxPointsPerLeaf = 4, int maxDepth = 3){
	QuadNode* root = new QuadNode();
	root->bounds = rootBounds;
	root->depth = 0;

	for (int i = 0; i < (int)positions.size(); i++) {
		insert(root, i, positions,maxPointsPerLeaf, maxDepth);
	}
	return root;
}

inline void freeQuadtree(QuadNode* node){
	if (node == nullptr) {
	 	return;
	}
	for(int i = 0; i < 4; i++){
		freeQuadtree(node->children[i]);
	}
	delete node;
}


inline bool squareIntersectsCircle(AABB2 square, Vector2 circleCenter, float radius){
  	float dx = std::max({square.center.x - square.halfSize - circleCenter.x, 0.0f, circleCenter.x - (square.center.x + square.halfSize)});
	float dy = std::max({square.center.y - square.halfSize - circleCenter.y, 0.0f, circleCenter.y - (square.center.y + square.halfSize)});
	return (dx*dx + dy*dy) <= radius*radius;
}

inline void queryRadius(QuadNode* node, Vector2 queryPos, float radius, const std::vector<Vector2>& positions, std::vector<int>& results){
	if (node == nullptr) {return;}
	if (!squareIntersectsCircle(node->bounds, queryPos, radius)) {return;}

	if(node->isLeaf()){
		for(int idx : node->PointIndices){
			float d2 = positions[idx].distance_squared_to(queryPos);
			if (d2 <= radius * radius){
				results.push_back(idx);
			}
		}
		return;
	}

	for (int i =0; i < 4; i++){
		queryRadius(node->children[i], queryPos, radius, positions, results);
	}
}


// Octtree

struct AABB3{
	Vector3 center;
	float halfSize;
};

struct OctNode{
	AABB3 bounds;
	std::vector<int> PointIndices;
	OctNode* children[8] = {nullptr};
	int depth = 0;
	bool isLeaf() const {return children[0]== nullptr;}
};


inline int getOctant(Vector3 point, Vector3 center){
	int q = 0;
	if (point.x >= center.x) {q |= 1;}
	if (point.y >= center.y) {q |= 2;}
	if (point.z >= center.z) {q |= 4;}
	return q;
}

inline Vector3 getChildCenters3D(Vector3 parentCenter, float parentHalfSize, int octant){
	float offset = parentHalfSize * 0.5f;
	Vector3 c = parentCenter;

	c.x += (octant & 1) ? offset : -offset;
	c.y += (octant & 2) ? offset : -offset;
	c.z += (octant & 4) ? offset : -offset;

	return c;
}

inline void subdivide(OctNode* node){
	for (int i =0; i < 8; i++){
		OctNode* child = new OctNode();
		child->bounds.center = getChildCenters3D(node->bounds.center, node->bounds.halfSize, i);
		child->bounds.halfSize = node->bounds.halfSize * 0.5f;
		child->depth = node->depth + 1;
		node->children[i] = child;
	}
}


inline void insert(OctNode* node, int pointIndex, const std::vector<Vector3>& positions, int maxPointsPerLeaf, int maxDepth){

	Vector3 pos = positions[pointIndex];

	if (node->isLeaf()) {
		node->PointIndices.push_back(pointIndex);

		if (shouldSplit((int)node->PointIndices.size(), node->depth,maxPointsPerLeaf, maxDepth)){
			subdivide(node);
			for(int idx : node->PointIndices){
				int octant = getOctant(positions[idx], node->bounds.center);
				insert(node->children[octant], idx, positions, maxPointsPerLeaf,maxDepth);
			}
			node->PointIndices.clear();
		}
	return;
	}
	int octant = getOctant(pos, node->bounds.center);
	insert(node->children[octant], pointIndex, positions, maxPointsPerLeaf, maxDepth);
}

inline OctNode* buildOctree(const std::vector<Vector3>& positions, AABB3 rootBounds, int maxPointsPerLeaf = 8, int maxDepth = 6){
	OctNode* root = new OctNode();
	root->bounds = rootBounds;
	root->depth = 0;

	for (int i = 0; i < (int)positions.size(); i++) {
		insert(root, i, positions,maxPointsPerLeaf, maxDepth);
	}
	return root;
}

inline void freeOctree(OctNode* node){
	if (node == nullptr) {
	 	return;
	}
	for(int i = 0; i < 8; i++){
		freeOctree(node->children[i]);
	}
	delete node;
}

inline bool cubeIntersectsSphere(AABB3 cube, Vector3 sphereCenter, float radius){
  	float dx = std::max({cube.center.x - cube.halfSize - sphereCenter.x, 0.0f, sphereCenter.x - (cube.center.x + cube.halfSize)});
	float dy = std::max({cube.center.y - cube.halfSize - sphereCenter.y, 0.0f, sphereCenter.y - (cube.center.y + cube.halfSize)});
	float dz = std::max({cube.center.z - cube.halfSize - sphereCenter.z, 0.0f, sphereCenter.z - (cube.center.z + cube.halfSize)});
	return (dx*dx + dy*dy + dz*dz) <= radius*radius;
}


inline void queryRadius(OctNode* node, Vector3 queryPos, float radius, const std::vector<Vector3>& positions, std::vector<int>& results){
	if (node == nullptr) {return;}
	if (!cubeIntersectsSphere(node->bounds, queryPos, radius)) {return;}

	if(node->isLeaf()){
		for(int idx : node->PointIndices){
			float d2 = positions[idx].distance_squared_to(queryPos);
			if (d2 <= radius * radius){
				results.push_back(idx);
			}
		}
		return;
	}

	for (int i =0; i < 8; i++){
		queryRadius(node->children[i], queryPos, radius, positions, results);
	}
}
#endif
