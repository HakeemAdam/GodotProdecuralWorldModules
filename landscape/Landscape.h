#pragma once
#include "core/object/ref_counted.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/surface_tool.h"
#include "modules/noise/fastnoise_lite.h"

class Landscape : public MeshInstance3D{
	GDCLASS(Landscape, MeshInstance3D)

	private:
		// Mesh, material, noise, mesh setting
		// getters and setters: size, division, material + settings
		Ref<Mesh> mesh;

		Ref<SurfaceTool> st;

		Ref<ShaderMaterial> landscape_material;

		Ref<FastNoiseLite> noise_instance;

		Ref<Texture2D> landscape_texture;

		bool p_use_texture;

		// Mesh variables
		Vector2 p_landscape_size;

		float p_noise_scale;

		float p_spacing;

		void create_collision();

		// Brush
		//MeshInstance3D* brush_mesh=nullptr;

	protected:
		static void _bind_methods();

		void _notification(int p_what);

	public:
		Landscape();
		~Landscape();

		void ready();

		void generate_landscape();


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





};
