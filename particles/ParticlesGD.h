#pragma once

#include "core/math/vector3.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "modules/procedural_world/particles/particles.h"

class ParticlesGD : public RefCounted{
	GDCLASS(ParticlesGD, RefCounted);

	private:

		ParticleSystem* p_system = nullptr;
		float dragCoeff = 0.15f;
		float separationDist = 0.75f;
		Vector3 gravity = {0.0f, -9.8f, 0.0f};
		float separationWeight = 1.5f;
		float cohesionWeight = 0.2f;
		float alignmentWeight = 0.5f;
		float restitution = 0.5f;
		float max_speed = 5.0f;
		float max_force = 2.0f;

	public:
		ParticlesGD();
		~ParticlesGD();



		void spawn(int count, Vector3 minBound, Vector3 maxBound, float lifeTime, Color clr, float radius = 2.0f);

		void update(float dt);

		PackedVector3Array getParticlePositions();

		Vector3 get_gravity();
		void set_gravity(Vector3 p_gravity);

		float get_drag_coeff() const;
		void set_drag_coeff(float p_value);

		float get_separation_dist() const;
		void set_separtion_dist(float p_value);

		float get_separation_weight() const;
		void set_separation_weight(float p_value);

		float get_cohesion_weight() const;
		void set_cohesion_weight(float p_value);

		float get_alignment_weight() const;
		void set_alignment_weight(float p_value);

		float get_restitution() const;
		void set_restitution(float p_value);

		float get_max_speed() const;
		void set_max_speed(float p_value);

		float get_max_force() const;
		void set_max_force(float p_value);


	protected:
		static void _bind_methods();
};
