#pragma once
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/surface_tool.h"
#include "modules/noise/fastnoise_lite.h"

class Landscape : public MeshInstance3D{
	GDCLASS(Landscape, MeshInstance3D)

	private:

		// Mesh Resources
		Ref<Mesh> mesh;
		Ref<SurfaceTool> st;
		Ref<ShaderMaterial> landscape_material;
		Ref<FastNoiseLite> noise_instance;
		Ref<Texture2D> landscape_texture;
		bool p_use_texture;
		Vector2 p_landscape_size;
		float p_noise_scale;
		float p_spacing;

		// Mesh Creation
		void create_collision();
		void initialize_height();
		void build_mesh();

		// Erosion
		void start_erosion_sim();
		void erode(int p_drops_per_frame);

		int num_droplets;
		bool use_erosion;
		int max_steps;
		bool is_eroding;
		bool is_paused;
		int current_droplet;
		float mesh_update_timer;
		const float MESH_UPDATE_INTERVAL= 0.1f;
		int drops_per_frame;

		PackedFloat32Array height_map;

	protected:
		static void _bind_methods();
		void _notification(int p_what);


	public:
		Landscape();
		~Landscape();

		void ready();

		// Signals
		void _on_noise_changed();

		// Mesh settings
		Vector2 get_landscape_size();
 		void set_landscape_size(const Vector2& p_size);

		float get_noise_scale();
		void set_noise_scale(const float p_scale);

		float get_spacing();
		void set_spacing(const float p_space);

		// Material Assignment
		Ref<ShaderMaterial> get_landscape_material();
		void set_landscape_material(const Ref<ShaderMaterial>& p_material);

		// Noise Assignment
		Ref<FastNoiseLite> get_landscape_noise();
		void set_landscape_noise(const Ref<FastNoiseLite>& p_noise);

		// Texture Assigment
		Ref<Texture2D> get_landscape_texture();
		void set_landscape_texture(const Ref<Texture2D>& p_texture);

		bool get_use_texture();
		void set_use_texture(const bool p_set);

		bool get_use_erosion();
		void set_use_erosion(const bool p_set);

		int get_max_steps();
		void set_max_steps(const int p_step);

		void set_run_simulation(bool p_run);
		bool get_run_simulation() const;

		void set_pause_simulation(bool p_pause);
		bool get_pause_simulation() const;

		void set_reset_and_clear(bool p_reset);
		bool get_reset_and_clear() const;


};
