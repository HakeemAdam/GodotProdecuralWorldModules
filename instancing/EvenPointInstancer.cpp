
#include "core/config/engine.h"
#include "core/math/aabb.h"
#include "core/math/basis.h"
#include "core/math/random_number_generator.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/property_info.h"
#include "core/object/ref_counted.h"
#include "core/os/memory.h"
#include "core/string/node_path.h"
#include "core/string/print_string.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/3d/physics/collision_object_3d.h"
#include "scene/3d/physics/collision_shape_3d.h"
#include "scene/main/node.h"
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/multimesh.h"
#include "servers/physics_3d/direct_states/physics_direct_space_state_3d.h"
#include "servers/physics_3d/physics_server_3d_types.h"
#include <cstdlib>
#include "EvenPointInstancer.h"
#include "PoissonDiskSample.h"

EvenPointInstancer::EvenPointInstancer(){
	count = 10;
	actual_count = 0;
	minDist = 2.0;
	maxAttempts = 30;
	bounds = {0,0, 25, 25};
	pointPositions = {};
	useTargetMesh = false;
	randomize = false;
	range_min = {0.0, 0.0, 0.0};
	range_max = {25.0, 25.0, 25.0};
	collision_mask = 1;
	scale_range = {1.0, 1.2};

	meshes = TypedArray<Mesh>();
	containers = TypedArray<MultiMesh>();
	occluded = TypedArray<NodePath>();

	rng.instantiate();
}


EvenPointInstancer::~EvenPointInstancer(){}

void EvenPointInstancer::_notification(int p_what){
	switch(p_what){
		case NOTIFICATION_READY : {
			ready();
		}
		break;
	}
}

// Vestige of gd extension, keeping for compatibility
void EvenPointInstancer::ready(){
    instance();
}

void EvenPointInstancer::calculate_positions(){
	if (!is_inside_tree()) {
        return;
    }

	actual_count = 0;
	pointPositions.clear();
	pointNormals.clear();

	bounds = {range_min.x, range_min.z, range_max.x, range_max.z};

	// Get evenly spaced positions
	PoissonInput input = {bounds, minDist, maxAttempts};
	PoissonOutput output;
	GeneratePoissonSampling(input,output);


	actual_count = std::min(count, static_cast<int>(output.points.size()));

	// Set evenly spaced or randomize
	for (int i = 0; i < actual_count; i++){
		Vector2 point = output.points[i];
		Vector3 pos;

		if(!randomize){
			pos = {point.x, range_max.y, point.y};
		}else{
			pos = {point.x, rng->randf_range(range_min.y, range_max.y), point.y};
		}
		pointPositions.push_back(pos);
		pointNormals.push_back(Vector3(0,1,0));
	}

	// Ray casting and occlusion
	if (useTargetMesh) {
		raycastPoints(target_mesh, pointPositions, pointNormals);
		remove_points();
	}
}

void EvenPointInstancer::manage_multis(){
	containers.clear();

	if (meshes.is_empty()) {
		return;
	}

	// Add mutlimesh instance for each mesh and set mesh
	for (int i=0; i < meshes.size(); i++) {
		Ref<MultiMesh> mm;
		Ref<Mesh> raw_mesh = meshes[i];

		if (raw_mesh.is_null()) {
			continue;
		}
		mm.instantiate();

		mm->set_mesh(raw_mesh);
		mm->set_transform_format(MultiMesh::TRANSFORM_3D);
		containers.push_back(mm);

	}
}

void EvenPointInstancer::instance(){
	// Remove old children before instancing
	for (int i = get_child_count() - 1; i >= 0; i--) {
		Node* child = get_child(i);
		if (Object::cast_to<MultiMeshInstance3D>(child)) {
			remove_child(child);
			memdelete(child);
		}
	}

	// Get positions and multis
	calculate_positions();
	manage_multis();

	if (containers.is_empty() || actual_count <= 0) {
		return;
	}

	// Diving containers/meshes based on number of meshes
	// Extend to support ratios
	for (int i = 0; i < containers.size(); i++) {
		Ref<MultiMesh> m = containers[i];
		m->set_instance_count(actual_count / containers.size());
		m->set_custom_aabb(AABB(range_min, range_max));
	}


	// Set transforms and normals based on division
	for (int i = 0; i < actual_count; i++){

		int mesh_idx = i % containers.size();
		int instance_idx = i / containers.size();

		Ref<MultiMesh> m = containers[mesh_idx];

		if(instance_idx < m->get_instance_count()){
			Transform3D instance_transform;
			instance_transform.origin = pointPositions[i];
			instance_transform.basis = Basis::from_euler(pointNormals[i]);
			instance_transform.basis *= rng->randf_range(scale_range.x, scale_range.y);
			m->set_instance_transform(instance_idx, instance_transform);
		}
	}

	// Create multi mesh instances and set multies to them and add to node tree. Do this last to prevent crashes
	for (int i = 0; i < containers.size(); i++) {
		MultiMeshInstance3D* mmi = memnew(MultiMeshInstance3D);
		mmi->set_multimesh(containers[i]);
		add_child(mmi);

		if (is_inside_tree() && Engine::get_singleton()->is_editor_hint()) {
			mmi->set_owner(get_owner() ? get_owner() : this);
		}
	}

}


void EvenPointInstancer::remove_points(){
	// Tracked valid positions and normals
	PackedVector3Array valid_pos;
	PackedVector3Array valid_normals;

	// Store bounding boxes of all occluders. So as to not recreate
	Vector<AABB> bounds_list;
	for(int i=0; i< occluded.size(); i++){
		if (has_node(occluded[i])) {
			MeshInstance3D* mesh = Object::cast_to<MeshInstance3D>(get_node(occluded[i]));
			if(mesh){
				bounds_list.push_back(mesh->get_global_transform().xform(mesh->get_aabb()));
			}
		}
	}

	// Check each point intersection
	for(int j=0; j < pointPositions.size(); j++){
		Vector3 globalPos = to_global(pointPositions[j]);
		bool insideAny = false;
		for(int k =0; k < bounds_list.size(); k++){
			if(bounds_list[k].has_point(globalPos)){
				insideAny = true;
				break;
			}
		}

		// Save valid positions and normals
		if(!insideAny){
			valid_pos.push_back(pointPositions[j]);
			valid_normals.push_back(pointNormals[j]);
		}
	}

	// Update positions and normals and instance count
	pointPositions = valid_pos;
	pointNormals = valid_normals;
	actual_count =pointPositions.size();
}


void EvenPointInstancer::raycastPoints(MeshInstance3D* target, PackedVector3Array& points, PackedVector3Array& normals){
	if (!target) {return;}

	if (!is_inside_world()) {
		return;
	}

	Ref<World3D> world = get_world_3d();
	if (world.is_null()) {return;}

	PhysicsDirectSpaceState3D* space_state = world->get_direct_space_state();
	if(!space_state){
		print_error("Phsyics stat unavailable");
	}

	float padding = 50.0f;
	float start_y = range_max.y + padding;
	float ray_length = (range_max.y - range_min.y) + padding + 100.0f;

	Basis go = get_global_transform().basis.inverse();

	for(int i = 0; i < points.size(); i++){
		Vector3 origin = to_global(Vector3(points[i].x, start_y, points[i].z));
		Vector3 dest = origin + Vector3(0.0, -ray_length, 0.0);


		PhysicsServer3DTypes::RayParameters params;
		params.from = origin;
		params.to = dest;
		params.collide_with_areas = true;
		params.collide_with_bodies = true;
		params.collision_mask = collision_mask;

		PhysicsServer3DTypes::RayResult results = {};

		// Align points with normals

		if(space_state->intersect_ray(params, results)){
			Vector3 hit_point = results.position;
			//print_line("Point : ", i , "pos: ", hit_point);

			Vector3 local_pos = to_local(hit_point);
			points.set(i, local_pos);

			Vector3 up = go.xform(results.normal).normalized();
			Vector3 tmp = (abs(up.dot(Vector3(0,1,0))) > 0.99f) ? Vector3(0, 0, -1) : Vector3(0, 1, 0);
			Vector3 right = tmp.cross(up).normalized();
			Vector3 forward = right.cross(up).normalized();

			Transform3D instance_transform;
			instance_transform.origin = local_pos;
			instance_transform.basis.set_column(0, right);
			instance_transform.basis.set_column(1, up);
			instance_transform.basis.set_column(2, forward);
			normals.set(i, instance_transform.basis.get_euler());
		}
	}
}


MeshInstance3D* EvenPointInstancer::get_target_mesh(){
	return target_mesh;
}

void EvenPointInstancer::set_target_mesh(MeshInstance3D* p_mesh){
	target_mesh = p_mesh;
	instance();
}

bool EvenPointInstancer::get_useTargetMesh(){
	return useTargetMesh;
}

void EvenPointInstancer::set_useTargetMesh(bool p_option){
	useTargetMesh = p_option;
	instance();
}

bool EvenPointInstancer::get_randomize(){
	return randomize;
}

void EvenPointInstancer::set_randomize(bool p_option){
	randomize = p_option;
	instance();
}

int EvenPointInstancer::get_count(){
	return count;
}

void EvenPointInstancer::set_count(int p_count){
	count = p_count;
	instance();
}

int EvenPointInstancer::get_collision_mask(){
	return collision_mask;
}

void EvenPointInstancer::set_collision_mask(int p_mask){
	collision_mask = p_mask;
	instance();
}

float EvenPointInstancer::get_minDist(){
	return minDist;
}

void EvenPointInstancer::set_minDist(float p_minDist){
	minDist = p_minDist;
	instance();
}


Vector3 EvenPointInstancer::get_range_min(){
	return range_min;
}

void EvenPointInstancer::set_range_min(Vector3 p_range){
	range_min = p_range;
	instance();
}

Vector3 EvenPointInstancer::get_range_max(){
	return range_max;
}

void EvenPointInstancer::set_range_max(Vector3 p_range){
	range_max = p_range;
	instance();
}


TypedArray<Mesh> EvenPointInstancer::get_meshes(){
	return meshes;
}

void EvenPointInstancer::set_meshes(Array p_meshes){
	meshes = p_meshes;
	instance();
}


TypedArray<NodePath > EvenPointInstancer::get_occluders(){

	return occluded;
}

void EvenPointInstancer::set_occluders(Array p_occluders){
	occluded = p_occluders;
	instance();
}

Vector2 EvenPointInstancer::get_scale_range(){
	return scale_range;
}

void EvenPointInstancer::set_scale_range(Vector2 p_scale_range){
	scale_range = p_scale_range;
	instance();
}


void EvenPointInstancer::_bind_methods(){

	ClassDB::bind_method(D_METHOD("get_target_mesh"), &EvenPointInstancer::get_target_mesh);

	ClassDB::bind_method(D_METHOD("set_target_mesh", "p_mesh"), &EvenPointInstancer::set_target_mesh);

	ClassDB::bind_method(D_METHOD("get_useTargetMesh"), &EvenPointInstancer::get_useTargetMesh);

	ClassDB::bind_method(D_METHOD("set_useTargetMesh", "p_option"), &EvenPointInstancer::set_useTargetMesh);

	ClassDB::bind_method(D_METHOD("get_randomize"), &EvenPointInstancer::get_randomize);

	ClassDB::bind_method(D_METHOD("set_randomize", "p_option"), &EvenPointInstancer::set_randomize);

	ClassDB::bind_method(D_METHOD("set_count", "p_count"), &EvenPointInstancer::set_count);

	ClassDB::bind_method(D_METHOD("get_count"), &EvenPointInstancer::get_count);

	ClassDB::bind_method(D_METHOD("set_collision_mask", "p_mask"), &EvenPointInstancer::set_collision_mask);

	ClassDB::bind_method(D_METHOD("get_collision_mask"), &EvenPointInstancer::get_collision_mask);

	ClassDB::bind_method(D_METHOD("set_minDist", "p_minDist"), &EvenPointInstancer::set_minDist);

	ClassDB::bind_method(D_METHOD("get_minDist"), &EvenPointInstancer::get_minDist);

	ClassDB::bind_method(D_METHOD("set_range_min", "p_range"), &EvenPointInstancer::set_range_min);

	ClassDB::bind_method(D_METHOD("get_range_min"), &EvenPointInstancer::get_range_min);

	ClassDB::bind_method(D_METHOD("set_range_max", "p_range"), &EvenPointInstancer::set_range_max);

	ClassDB::bind_method(D_METHOD("get_range_max"), &EvenPointInstancer::get_range_max);

	ClassDB::bind_method(D_METHOD("set_meshes", "p_mehses"), &EvenPointInstancer::set_meshes);

	ClassDB::bind_method(D_METHOD("get_meshes"), &EvenPointInstancer::get_meshes);

	ClassDB::bind_method(D_METHOD("get_occluders"), &EvenPointInstancer::get_occluders);

	ClassDB::bind_method(D_METHOD("set_occluders", "p_occluders"), &EvenPointInstancer::set_occluders);

	ClassDB::bind_method(D_METHOD("get_scale_range"), &EvenPointInstancer::get_scale_range);

	ClassDB::bind_method(D_METHOD("set_scale_range", "p_scale_range"), &EvenPointInstancer::set_scale_range);

	// Properties

	ADD_PROPERTY(PropertyInfo(Variant::INT, "Count"), "set_count", "get_count");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Minimum Distance"), "set_minDist", "get_minDist");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "Scale Range"), "set_scale_range", "get_scale_range");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Randomize"), "set_randomize", "get_randomize");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Use Target Mesh"), "set_useTargetMesh", "get_useTargetMesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Target Mesh", PROPERTY_HINT_NODE_TYPE, "MeshInstance3D"), "set_target_mesh","get_target_mesh");


	ADD_PROPERTY(PropertyInfo(Variant::INT, "Collision Mask"), "set_collision_mask", "get_collision_mask");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "Range Min"), "set_range_min", "get_range_min");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "Range Max"), "set_range_max", "get_range_max");

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "Instance Meshes", PROPERTY_HINT_TYPE_STRING, "Mesh"), "set_meshes", "get_meshes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "Occluders", PROPERTY_HINT_TYPE_STRING, vformat("%d/%d:%s", Variant::NODE_PATH, PROPERTY_HINT_NODE_TYPE, "MeshInstance3D")), "set_occluders","get_occluders");

}
