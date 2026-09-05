
#include "core/math/random_number_generator.h"
#include "core/object/class_db.h"
#include "scene/main/node.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "RandomInstancer.h"


void RandomInstancer::_bind_methods(){
	ClassDB::bind_method(D_METHOD("get_instance_mesh"), &RandomInstancer::get_instance_mesh);

	ClassDB::bind_method(D_METHOD("set_instance_mesh", "p_mesh"), &RandomInstancer::set_instance_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instance_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh", PROPERTY_USAGE_DEFAULT), "set_instance_mesh","get_instance_mesh");

	ClassDB::bind_method(D_METHOD("get_count"), &RandomInstancer::get_count);

	ClassDB::bind_method(D_METHOD("set_count", "p_count"), &RandomInstancer::set_count);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "count"), "set_count", "get_count");

	ClassDB::bind_method(D_METHOD("get_range_x"), &RandomInstancer::get_range_x);

	ClassDB::bind_method(D_METHOD("set_range_x", "p_range"), &RandomInstancer::set_range_x);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "range_x"), "set_range_x", "get_range_x");

	ClassDB::bind_method(D_METHOD("get_range_y"), &RandomInstancer::get_range_y);

	ClassDB::bind_method(D_METHOD("set_range_y", "p_range"), &RandomInstancer::set_range_y);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "range_y"), "set_range_y", "get_range_y");

	ClassDB::bind_method(D_METHOD("get_range_z"), &RandomInstancer::get_range_z);

	ClassDB::bind_method(D_METHOD("set_range_z", "p_range"), &RandomInstancer::set_range_z);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "range_z"), "set_range_z", "get_range_z");
}

RandomInstancer::RandomInstancer(){
	count = 10;
	range_x = {0.0, 10.0};
	range_y = {0.0, 10.0};
	range_z = {0.0, 10.0};

}

RandomInstancer::~RandomInstancer(){}

void RandomInstancer::_notification(int p_what){
	switch (p_what) {
		case NOTIFICATION_READY : {
			ready();
		}
		break;
	}
}


void RandomInstancer::ready(){

	if (!multi_mesh.is_valid()) {
        	multi_mesh.instantiate();
        	multi_mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
        	set_multimesh(multi_mesh);
    	}


	if(!instance_mesh.is_valid()){
		Ref<BoxMesh> box;
		box.instantiate();
		instance_mesh = box;
		multi_mesh->set_mesh(instance_mesh);

	}

    	multi_mesh->set_mesh(instance_mesh);
    	instance();

}


void RandomInstancer::instance(){

	if (!multi_mesh.is_valid() || !instance_mesh.is_valid()) {
		return;
	}


	multi_mesh->set_transform_format(MultiMesh::TRANSFORM_3D);
	multi_mesh->set_custom_aabb(AABB(Vector3(range_x.x, range_y.x, range_z.x), Vector3(range_x.y, range_y.y, range_z.y)));

	multi_mesh->set_instance_count(count);

	// alsways use godot ref for objects

	Ref<RandomNumberGenerator> rng;
	rng.instantiate();

	for( int i =0; i < count; i++ ){
		Vector3 pos = {rng->randf_range(range_x.x, range_x.y),rng->randf_range(range_y.x, range_y.y) , rng->randf_range(range_z.x, range_z.y) };
		Transform3D instance_transform;

		instance_transform.origin = pos;

		multi_mesh->set_instance_transform(i, instance_transform);
	}
}


Ref<Mesh> RandomInstancer::get_instance_mesh(){
	return instance_mesh;
}

void RandomInstancer::set_instance_mesh(Ref<Mesh> p_mesh){
	instance_mesh = p_mesh;
	if (multi_mesh.is_valid()) {
		multi_mesh->set_mesh(instance_mesh);

	}
}

int RandomInstancer::get_count(){
	return count;
}

void RandomInstancer::set_count(int p_count){
	count = p_count;
	instance();
}

Vector2 RandomInstancer::get_range_x(){
	return range_x;
}

void RandomInstancer::set_range_x(Vector2 p_range){
	range_x = p_range;
	instance();
}


Vector2 RandomInstancer::get_range_y(){
	return range_y;
}

void RandomInstancer::set_range_y(Vector2 p_range){
	range_y = p_range;
	instance();
}

Vector2 RandomInstancer::get_range_z(){
	return range_z;
}

void RandomInstancer::set_range_z(Vector2 p_range){
	range_z = p_range;
	instance();
}
