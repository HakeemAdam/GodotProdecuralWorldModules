#pragma once

#include "GridOps.h"
#include "core/math/vector3.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/resources/mesh.h"
#include <vector>



class EvenPointInstancer : public MultiMeshInstance3D{
	GDCLASS(EvenPointInstancer, MultiMeshInstance3D);

	private:
		float minDist;
		int maxAttempts;
		Bounds2D bounds;

		Ref<MultiMesh> multi_mesh;

		Ref<Mesh> instance_mesh;

		int count;

		MeshInstance3D* target_mesh = nullptr;
		bool useTargetMesh;
		bool randomize;

		// add parms for bounds and min dist
		// add mesh for ray casting
		// Rename class
		// replace std vector with packedarray or engine equivalent
		void raycastPoints(MeshInstance3D* target, std::vector<Vector3>& points);

		std::vector<Vector3> pointPositions;
		Vector3 range_min;
		Vector3 range_max;

	protected:
		static void _bind_methods();
		void _notification(int p_what);

	public:
		EvenPointInstancer();
		~EvenPointInstancer();

		void ready();

		void instance();

		Ref<Mesh> get_instance_mesh();
		void set_instance_mesh(Ref<Mesh> p_mesh);

		MeshInstance3D* get_target_mesh();
		void set_target_mesh(MeshInstance3D* p_mesh);

		bool get_useTargetMesh();
		void set_useTargetMesh(bool p_option);

		bool get_randomize();
		void set_randomize(bool p_option);

		int get_count();
		void set_count(int p_count);

		float get_minDist();
		void set_minDist(float p_minDist);

		Vector3 get_range_min();
		void set_range_min(Vector3 p_range);

		Vector3 get_range_max();
		void set_range_max(Vector3 p_range);
};

