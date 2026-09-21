#pragma once

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "modules/procedural_world/particles/particles.h"

class ParticlesGD : public RefCounted{
	GDCLASS(ParticlesGD, RefCounted);

	private:

		ParticleSystem* p_system = nullptr;

	public:
		ParticlesGD();
		~ParticlesGD();

		void spawn(int count, Vector3 minBound, Vector3 maxBound, float lifeTime, Color clr, float radius = 2.0f);

		void update(float dt);

		PackedVector3Array getParticlePositions();

	protected:
		static void _bind_methods();
};
