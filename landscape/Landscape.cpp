#include "Landscape.h"

#include "core/math/random_number_generator.h"
#include "core/math/vector2.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/property_info.h"
#include "core/object/ref_counted.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

#include "modules/noise/fastnoise_lite.h"

#include <algorithm>

Landscape::Landscape() {
	p_landscape_size = Vector2(32, 32);
	p_noise_scale = 10.0f;
	p_spacing = 1.0f;
	p_use_texture = false;
	num_droplets = 10000;
	use_erosion = false;
	is_eroding = false;
	is_paused = false;
	current_droplet = 0;
	mesh_update_timer = 0.0f;
	drops_per_frame = 50;
	max_steps = 1000;
	st.instantiate();

	// Sim settings
	capacity_factor = 0.1;
	min_capacity = 0.5;
	erosion_speed = 0.005;
	evaporation_rate = 0.001;
}

Landscape::~Landscape() {}

void Landscape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process(true);
			ready();
		} break;

		case NOTIFICATION_PROCESS: {
			if (is_eroding && !is_paused) {
				double delta = get_process_delta_time();
				erode(drops_per_frame);
				mesh_update_timer += delta;
				if (mesh_update_timer >= MESH_UPDATE_INTERVAL) {
					build_mesh();
					mesh_update_timer = 0.0f;
				}
			}
			break;
		}
	}
	Node::_notification(p_what);
}

void Landscape::ready() {
	if (!landscape_material.is_valid()) {
		landscape_material.instantiate();
		set_material_override(landscape_material);
	}

	if (!noise_instance.is_valid()) {
		noise_instance.instantiate();
		noise_instance->connect("changed", Callable(this, "_on_noise_changed"));
	}
	initialize_height();
	if (use_erosion) {
		start_erosion_sim();
	} else {
		build_mesh();
	}
}

void Landscape::start_erosion_sim() {
	current_droplet = 0;
	is_eroding = true;
	mesh_update_timer = 0.0;
	drops_per_frame = 50;
}

// PERF: Consider caching height
void Landscape::initialize_height() {
	int width = (int)p_landscape_size.x;
	int depth = (int)p_landscape_size.y;
	height_map.resize(width * depth);

	Ref<Image> img;
	if (landscape_texture.is_valid()) {
		img = landscape_texture->get_image();
		if (img.is_valid() && img->is_compressed()) {
			img->decompress();
		}
	}

	// Sample noise
	for (int z = 0; z < depth; z++) {
		for (int x = 0; x < width; x++) {
			float y = 0.0f;

			// Use texture
			if (p_use_texture == true) {
				if (img.is_valid()) {
					float u = (width > 1) ? (float)x / (width - 1) : 0.0f;
					float v = (depth > 1) ? (float)z / (depth - 1) : 0.0f;

					int img_x = (int)(u * (img->get_width() - 1));
					int img_z = (int)(v * (img->get_height() - 1));

					img_x = CLAMP(img_x, 0, img->get_width() - 1);
					img_z = CLAMP(img_z, 0, img->get_height() - 1);

					Color px_color = img->get_pixel(img_x, img_z);
					y = px_color.get_v() * p_noise_scale;
				}
			} else {
				// Use noise
				if (noise_instance.is_valid()) {
					y = noise_instance->get_noise_2d((float)x, (float)z) * p_noise_scale;
				}
			}
			height_map.set(x * depth + z, y);
		}
	}
}

void Landscape::build_mesh() {
	st->clear();
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	float spacing = p_spacing;
	int width = (int)p_landscape_size.x;
	int depth = (int)p_landscape_size.y;

	for (int z = 0; z < depth; z++) {
		for (int x = 0; x < width; x++) {
			// Grab the eroded height!
			float y = height_map[x * depth + z];

			st->set_uv(Vector2((float)x / (width - 1), (float)z / (depth - 1)));
			st->add_vertex(Vector3(x * spacing, y, z * spacing));
		}
	}

	// Build vetices
	for (int z = 0; z < depth - 1; z++) {
		for (int x = 0; x < width - 1; x++) {
			// Get vertex row indices
			int tl = x * depth + z;
			int tr = (x + 1) * depth + z;
			int bl = x * depth + (z + 1);
			int br = (x + 1) * depth + (z + 1);

			// Triangle 1
			st->add_index(tl);
			st->add_index(bl);
			st->add_index(tr);

			st->add_index(bl);
			st->add_index(br);
			st->add_index(tr);
		}
	}

	st->generate_normals();

	mesh = st->commit();
	set_mesh(mesh);
	set_material_override(landscape_material);

	// Collision
	// TODO: Replace with static member
	create_collision();
}

void Landscape::create_collision() {
	// Get children
	TypedArray<Node> children = get_children();

	// loop backwards and remove
	for (int i = children.size() - 1; i >= 0; i--) {
		Node *child = Object::cast_to<Node>(children[i]);

		if (child != nullptr && child->is_class("StaticBody3D")) {
			remove_child(child);
			child->queue_free();
		}
	}
	create_trimesh_collision();
}

void Landscape::erode(int p_drops_per_frame) {
	if (!is_eroding) {
		return;
	}

	int map_w = p_landscape_size.x;
	int map_h = p_landscape_size.y;

	struct Drop {
		Vector2 pos;
		Vector2 vel;
		float water_amt;
		float sediment_amt;
	};

	// TODO: Move rng to class member
	Ref<RandomNumberGenerator> rng;
	rng.instantiate();

	const float inertia = 0.1f;
	const float friction = 0.9f; // was the broken "0.5f * -2.0f" damping term

	int end_droplet = std::min(current_droplet + p_drops_per_frame, num_droplets);

	for (int i = current_droplet; i < end_droplet; i++) {
		Drop drop;
		drop.pos = Vector2(rng->randf_range(0, map_w - 1), rng->randf_range(0, map_h - 1));
		drop.water_amt = 1.0;
		drop.sediment_amt = 0.0;
		drop.vel = Vector2(0, 0);
		int steps = 0;

		while (drop.water_amt > 0 && steps < max_steps) {
			int gridX = (int)(drop.pos.x);
			int gridZ = (int)(drop.pos.y);

			if (gridX <= 0 || gridX >= map_w - 1 || gridZ <= 0 || gridZ >= map_h - 1) {
				break;
			}

			int right = ((gridX + 1) * map_h) + gridZ;
			int left  = ((gridX - 1) * map_h) + gridZ;
			int up    = (gridX * map_h) + (gridZ + 1);
			int down  = (gridX * map_h) + (gridZ - 1);
			int cur   = (gridX * map_h) + gridZ;

			float slopeX = (height_map[right] - height_map[left]) / 2.0f;
			float slopeZ = (height_map[up] - height_map[down]) / 2.0f;

			Vector2 slope = { -slopeX, -slopeZ };

			drop.vel = drop.vel * inertia + slope * (1.0f - inertia);
			drop.pos += drop.vel;
			drop.vel *= friction;

			// Capacity
			float speed = drop.vel.length();
			float max_capacity = speed * capacity_factor;

			if (drop.sediment_amt < max_capacity) {
				// Erode
				float erosion_amt = (max_capacity - drop.sediment_amt) * erosion_speed;
				height_map.set(cur, height_map[cur] - erosion_amt);
				drop.sediment_amt += erosion_amt;
			} else {
				// Deposit
				float deposit_amt = (drop.sediment_amt - max_capacity) * erosion_speed;
				height_map.set(cur, height_map[cur] + deposit_amt);
				drop.sediment_amt -= deposit_amt;
			}
			steps++;
			drop.water_amt -= evaporation_rate;
		}
	}

	current_droplet = end_droplet;

	if (current_droplet >= num_droplets) {
		is_eroding = false;
		build_mesh();
		print_line("Sim complete");
	}
}
void Landscape::_on_noise_changed() {
	ready();
}

Vector2 Landscape::get_landscape_size() {
	return p_landscape_size;
}

void Landscape::set_landscape_size(const Vector2 &p_size) {
	p_landscape_size = p_size;
	ready();
}

float Landscape::get_noise_scale() {
	return p_noise_scale;
}

void Landscape::set_noise_scale(const float p_scale) {
	p_noise_scale = p_scale;
	ready();
}

float Landscape::get_spacing() {
	return p_spacing;
}

void Landscape::set_spacing(const float p_space) {
	p_spacing = p_space;
	ready();
}

Ref<ShaderMaterial> Landscape::get_landscape_material() {
	return landscape_material;
}

void Landscape::set_landscape_material(const Ref<ShaderMaterial> &p_material) {
	landscape_material = p_material;
	ready();
}

Ref<FastNoiseLite> Landscape::get_landscape_noise() {
	return noise_instance;
}

void Landscape::set_landscape_noise(const Ref<FastNoiseLite> &p_noise) {
	if (noise_instance.is_valid() && noise_instance->is_connected("changed", Callable(this, "_on_noise_changed"))) {
		noise_instance->disconnect("changed", Callable(this, "_on_noise_changed"));
	}
	noise_instance = p_noise;

	if (noise_instance.is_valid()) {
		noise_instance->connect("changed", Callable(this, "_on_noise_changed"));
	}
	ready();
}

Ref<Texture2D> Landscape::get_landscape_texture() {
	return landscape_texture;
}

void Landscape::set_landscape_texture(const Ref<Texture2D> &p_texture) {
	landscape_texture = p_texture;
	ready();
}

bool Landscape::get_use_texture() {
	return p_use_texture;
}

void Landscape::set_use_texture(const bool p_set) {
	p_use_texture = p_set;
	ready();
}

bool Landscape::get_use_erosion() {
	return use_erosion;
}

void Landscape::set_use_erosion(const bool p_set) {
	use_erosion = p_set;
	ready();
}

int Landscape::get_max_steps() {
	return max_steps;
}

void Landscape::set_max_steps(const int p_step) {
	max_steps = p_step;
	ready();
}

void Landscape::set_run_simulation(bool p_run) {
	if (p_run) {
		start_erosion_sim();
		is_paused = false;
	}
}
bool Landscape::get_run_simulation() const {
	return is_eroding;
}

void Landscape::set_pause_simulation(bool p_pause) {
	is_paused = p_pause;
}
bool Landscape::get_pause_simulation() const {
	return is_paused;
}

void Landscape::set_reset_and_clear(bool p_reset) {
	if (p_reset) {
		is_eroding = false;
		is_paused = false;
		initialize_height();
		build_mesh();
	}
}

bool Landscape::get_reset_and_clear() const {
	return false;
}

void Landscape::set_capacity_factor(float p_rate) {
	capacity_factor = p_rate;
}

float Landscape::get_capacity_factor() {
	return capacity_factor;
}

void Landscape::set_min_capacity(float p_capacity) {
	min_capacity = p_capacity;
}

float Landscape::get_min_capacity() {
	return min_capacity;
}

void Landscape::set_erosion_speed(float p_speed) {
	erosion_speed = p_speed;
	is_eroding = true;
}

float Landscape::get_erosion_speed() {
	return erosion_speed;
}

void Landscape::set_evaporation_rate(float p_rate) {
	evaporation_rate = p_rate;
}

float Landscape::get_evaporation_rate() {
	return evaporation_rate;
}

// Params

void Landscape::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_noise_changed"), &Landscape::_on_noise_changed);

	ClassDB::bind_method(D_METHOD("get_landscape_size"), &Landscape::get_landscape_size);
	ClassDB::bind_method(D_METHOD("set_landscape_size", "p_size"), &Landscape::set_landscape_size);

	ClassDB::bind_method(D_METHOD("get_noise_scale"), &Landscape::get_noise_scale);
	ClassDB::bind_method(D_METHOD("set_noise_scale", "p_scale"), &Landscape::set_noise_scale);

	ClassDB::bind_method(D_METHOD("get_spacing"), &Landscape::get_spacing);
	ClassDB::bind_method(D_METHOD("set_spacing", "p_space"), &Landscape::set_spacing);

	ClassDB::bind_method(D_METHOD("get_landscape_material"), &Landscape::get_landscape_material);
	ClassDB::bind_method(D_METHOD("set_landscape_material", "p_material"), &Landscape::set_landscape_material);

	ClassDB::bind_method(D_METHOD("get_landscape_noise"), &Landscape::get_landscape_noise);
	ClassDB::bind_method(D_METHOD("set_landscape_noise", "p_noise"), &Landscape::set_landscape_noise);

	ClassDB::bind_method(D_METHOD("get_landscape_texture"), &Landscape::get_landscape_texture);
	ClassDB::bind_method(D_METHOD("set_landscape_texture", "p_texture"), &Landscape::set_landscape_texture);

	ClassDB::bind_method(D_METHOD("get_use_texture"), &Landscape::get_use_texture);
	ClassDB::bind_method(D_METHOD("set_use_texture", "p_set"), &Landscape::set_use_texture);

	ClassDB::bind_method(D_METHOD("get_use_erosion"), &Landscape::get_use_erosion);
	ClassDB::bind_method(D_METHOD("set_use_erosion", "p_set"), &Landscape::set_use_erosion);

	ClassDB::bind_method(D_METHOD("get_max_steps"), &Landscape::get_max_steps);
	ClassDB::bind_method(D_METHOD("set_max_steps", "p_step"), &Landscape::set_max_steps);

	ClassDB::bind_method(D_METHOD("start_erosion_sim"), &Landscape::start_erosion_sim);
	ClassDB::bind_method(D_METHOD("initialize_heigth"), &Landscape::initialize_height);
	ClassDB::bind_method(D_METHOD("build_mesh"), &Landscape::build_mesh);
	ClassDB::bind_method(D_METHOD("set_run_simulation", "run"), &Landscape::set_run_simulation);
	ClassDB::bind_method(D_METHOD("get_run_simulation"), &Landscape::get_run_simulation);
	ClassDB::bind_method(D_METHOD("set_pause_simulation", "pause"), &Landscape::set_pause_simulation);
	ClassDB::bind_method(D_METHOD("get_pause_simulation"), &Landscape::get_pause_simulation);
	ClassDB::bind_method(D_METHOD("set_reset_and_clear", "reset"), &Landscape::set_reset_and_clear);
	ClassDB::bind_method(D_METHOD("get_reset_and_clear"), &Landscape::get_reset_and_clear);

	ClassDB::bind_method(D_METHOD("get_capacity_factor"), &Landscape::get_capacity_factor);
	ClassDB::bind_method(D_METHOD("set_capacity_factor", "p_rate"), &Landscape::set_capacity_factor);

	ClassDB::bind_method(D_METHOD("get_erosion_speed"), &Landscape::get_erosion_speed);
	ClassDB::bind_method(D_METHOD("set_erosion_speed", "p_speed"), &Landscape::set_erosion_speed);

	ClassDB::bind_method(D_METHOD("get_evaporation_rate"), &Landscape::get_evaporation_rate);
	ClassDB::bind_method(D_METHOD("set_evaporation_rate", "p_rate"), &Landscape::set_evaporation_rate);

	ClassDB::bind_method(D_METHOD("get_min_capacity"), &Landscape::get_min_capacity);
	ClassDB::bind_method(D_METHOD("set_min_capacity", "p_capacity"), &Landscape::set_min_capacity);

	// Param Groups

	ADD_GROUP("Landscape Properties", "");

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "landscape_size"), "set_landscape_size", "get_landscape_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_scale"), "set_noise_scale", "get_noise_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spacing"), "set_spacing", "get_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_texture"), "set_use_texture", "get_use_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "landscape_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_DEFAULT), "set_landscape_texture", "get_landscape_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "landscape_noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseLite", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_landscape_noise", "get_landscape_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "landscape_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_landscape_material", "get_landscape_material");

	ADD_GROUP("Simulation Controls", "");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use Erosion"), "set_use_erosion", "get_use_erosion");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "Max Steps"), "set_max_steps", "get_max_steps");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Capacity Factor"), "set_capacity_factor", "get_capacity_factor");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, " Min Capacity"), "set_min_capacity", "get_min_capacity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Erosion Speed"), "set_erosion_speed", "get_erosion_speed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Evaporation Rate"), "set_evaporation_rate", "get_evaporation_rate");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "run_simulation"), "set_run_simulation", "get_run_simulation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "pause_simulation"), "set_pause_simulation", "get_pause_simulation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_and_clear"), "set_reset_and_clear", "get_reset_and_clear");
}
