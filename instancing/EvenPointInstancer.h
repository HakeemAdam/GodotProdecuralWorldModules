#pragma once

#include "GridOps.h"
#include "core/math/vector3.h"
#include "core/object/object.h"
#include "core/string/node_path.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/multimesh.h"



class EvenPointInstancer : public Node3D{
	GDCLASS(EvenPointInstancer, Node3D);

	private:

		// Posisson Disk Variables
		float minDist;
		int maxAttempts;
		Bounds2D bounds;

		// Instancing
		int count;
		Vector3 range_min;
		Vector3 range_max;
		TypedArray<Mesh> meshes;
		MeshInstance3D* target_mesh = nullptr;
		bool useTargetMesh;
		bool randomize;
		int actual_count;
		int collision_mask;

		// Containers
		PackedVector3Array pointPositions;
		PackedVector3Array pointNormals;
		TypedArray<MultiMesh> containers;
		TypedArray<NodePath> occluded;

		// Internal functions
		void raycastPoints(MeshInstance3D* target, PackedVector3Array& points, PackedVector3Array& normals);
		void calculate_positions();
		void manage_multis();
		HashSet<RID> getOcclusionList();
		void remove_points();

	public:
		EvenPointInstancer();
		~EvenPointInstancer();

		void ready();
		void instance();

		// Bindings

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

		int get_collision_mask();
		void set_collision_mask(int p_mask);

		float get_minDist();
		void set_minDist(float p_minDist);

		Vector3 get_range_min();
		void set_range_min(Vector3 p_range);

		Vector3 get_range_max();
		void set_range_max(Vector3 p_range);

		TypedArray<Mesh> get_meshes();
		void set_meshes(Array p_meshes);

		TypedArray<NodePath> get_occluders();
		void set_occluders(Array p_occluders);

	protected:
		static void _bind_methods();
		void _notification(int p_what);
};

