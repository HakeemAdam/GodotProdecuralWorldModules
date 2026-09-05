#pragma once


#include "core/math/vector2.h"
#include "core/object/ref_counted.h"
#include "scene/3d/multimesh_instance_3d.h"
class RandomInstancer: public MultiMeshInstance3D{
	GDCLASS(RandomInstancer, MultiMeshInstance3D);

	private:
		Ref<Mesh> instance_mesh;
		Ref<MultiMesh> multi_mesh;

		int count;
		Vector2 range_x;
		Vector2 range_y;
		Vector2 range_z;



	protected:
		static void _bind_methods();

		void _notification(int p_what);

	public:
		RandomInstancer();
		~RandomInstancer();

		void ready();

		void instance();

		Ref<Mesh> get_instance_mesh();
		void set_instance_mesh(Ref<Mesh> p_mesh);

		int get_count();
		void set_count(int p_count);

		Vector2 get_range_x();
		void set_range_x(Vector2 p_range);

		Vector2 get_range_y();
		void set_range_y(Vector2 p_range);

		Vector2 get_range_z();
		void set_range_z(Vector2 p_range);
};
