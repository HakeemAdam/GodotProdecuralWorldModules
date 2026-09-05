
#include "core/math/vector3.h"
#include "core/object/class_db.h"
#include "core/object/ref_counted.h"
#include "core/string/print_string.h"
#include "core/variant/dictionary.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/3d/world_3d.h"
#include "servers/physics_3d/direct_states/physics_direct_space_state_3d.h"
#include "servers/physics_3d/physics_server_3d_types.h"
#include <vector>
#include "EvenPointInstancer.h"
#include "PoissonDiskSample.h"

void EvenPointInstancer::_bind_methods(){

	ClassDB::bind_method(D_METHOD("get_instance_mesh"), &EvenPointInstancer::get_instance_mesh);

	ClassDB::bind_method(D_METHOD("set_instance_mesh", "p_mesh"), &EvenPointInstancer::set_instance_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instance_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh", PROPERTY_USAGE_DEFAULT), "set_instance_mesh","get_instance_mesh");

	ClassDB::bind_method(D_METHOD("get_target_mesh"), &EvenPointInstancer::get_target_mesh);

	ClassDB::bind_method(D_METHOD("set_target_mesh", "p_mesh"), &EvenPointInstancer::set_target_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_mesh", PROPERTY_HINT_NODE_TYPE, "MeshInstance3D"), "set_target_mesh","get_target_mesh");

	ClassDB::bind_method(D_METHOD("get_useTargetMesh"), &EvenPointInstancer::get_useTargetMesh);

	ClassDB::bind_method(D_METHOD("set_useTargetMesh", "p_option"), &EvenPointInstancer::set_useTargetMesh);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use target Mesh"), "set_useTargetMesh", "get_useTargetMesh");

	ClassDB::bind_method(D_METHOD("set_count", "p_count"), &EvenPointInstancer::set_count);

	ClassDB::bind_method(D_METHOD("get_count"), &EvenPointInstancer::get_count);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "count"), "set_count", "get_count");

	ClassDB::bind_method(D_METHOD("set_minDist", "p_minDist"), &EvenPointInstancer::set_minDist);

	ClassDB::bind_method(D_METHOD("get_minDist"), &EvenPointInstancer::get_minDist);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min dist"), "set_minDist", "get_minDist");


}

EvenPointInstancer::EvenPointInstancer(){
	count = 10;
	minDist = 2.0;
	maxAttempts = 30;
	bounds = {0,0, 100, 100};
	pointPositions = {};
}


EvenPointInstancer::~EvenPointInstancer(){

}

void EvenPointInstancer::_notification(int p_what){
	switch(p_what){
		case NOTIFICATION_READY : {
			ready();
		}
		break;
	}
}

void EvenPointInstancer::ready(){

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

void EvenPointInstancer::instance(){

	if (!multi_mesh.is_valid() || !instance_mesh.is_valid()) {
		return;
	}
	std::vector<Vector2> outputPoints;
	pointPositions.clear();

	PoissonInput input = {bounds, minDist, maxAttempts};
	PoissonOutput output = {outputPoints};

	GeneratePoissonSampling(input,output);
	multi_mesh->set_custom_aabb(AABB(Vector3(0,0,0), Vector3(50, 50, 50)));;
	int actual_count = std::min(count, (int)output.points.size());
	multi_mesh->set_instance_count(actual_count);

	for (int i = 0; i < actual_count; i++){
		Vector2 point = output.points[i];
		Vector3 pos = {point.x, 10.0, point.y};

		Transform3D instance_transform;

		instance_transform.origin = pos;
		multi_mesh->set_instance_transform(i, instance_transform);
		pointPositions.push_back(pos);
	}

	raycastPoints(target_mesh, pointPositions);

	// ray cast points onto surface
	// think about occlusions
}

void EvenPointInstancer::raycastPoints(MeshInstance3D* target, std::vector<Vector3>& points){
	if (!target) {return;}

	Ref<World3D> world = get_world_3d();
	if (world.is_null()) {return;}

	PhysicsDirectSpaceState3D* space_state = world->get_direct_space_state();
	if(!space_state){
		print_error("Phsyics stat unavailable");
	}

	for(int i = 0; i < points.size(); i++){
		Vector3 origin = points[i];
		Vector3 dest = origin + Vector3(0.0, -100.0, 0.0);

		PhysicsServer3DTypes::RayParameters params;
		params.from = origin;
		params.to = dest;
		params.collide_with_areas = true;
		params.collide_with_bodies = true;

		PhysicsServer3DTypes::RayResult results = {};

		if(space_state->intersect_ray(params, results)){
			Vector3 hit_point = results.position;
			print_line("Point : ", i , "pos: ", hit_point);
			Transform3D instance_transform;
			instance_transform.origin = hit_point;
			multi_mesh->set_instance_transform(i, instance_transform);

			}else{
				print_line("No hit");
			}
	}
}



Ref<Mesh> EvenPointInstancer::get_instance_mesh(){
	return instance_mesh;
}

void EvenPointInstancer::set_instance_mesh(Ref<Mesh> p_mesh){
	instance_mesh = p_mesh;
	if (multi_mesh.is_valid()) {
		multi_mesh->set_mesh(instance_mesh);

	}
}

MeshInstance3D* EvenPointInstancer::get_target_mesh(){
	return target_mesh;
}

void EvenPointInstancer::set_target_mesh(MeshInstance3D* p_mesh){
	target_mesh = p_mesh;
}

bool EvenPointInstancer::get_useTargetMesh(){
	return useTargetMesh;
}

void EvenPointInstancer::set_useTargetMesh(bool p_option){
	useTargetMesh = p_option;
}

int EvenPointInstancer::get_count(){
	return count;
}

void EvenPointInstancer::set_count(int p_count){
	count = p_count;
	instance();
}

float EvenPointInstancer::get_minDist(){
	return minDist;
}

void EvenPointInstancer::set_minDist(float p_minDist){
	minDist = p_minDist;
	instance();
}


