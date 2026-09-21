#include "ParticlesGD.h"
#include "core/object/class_db.h"
#include "modules/procedural_world/particles/particles.h"
#include <cstddef>

ParticlesGD::ParticlesGD(){
	p_system = new ParticleSystem();
}

ParticlesGD::~ParticlesGD(){
		delete p_system;
}

void ParticlesGD::spawn(int count, Vector3 minBound, Vector3 maxBound, float lifeTime, Color clr, float radius){
	if (!p_system) {return;}
	p_system->spawn(count, minBound, maxBound, lifeTime, clr);
}

void ParticlesGD::update(float dt){
	if (!p_system) {return;}
	p_system->update(dt);
}


PackedVector3Array ParticlesGD::getParticlePositions(){
	PackedVector3Array results;

    if (!p_system) {
        return results;
    }

    std::vector<Particle> particles = p_system->getParticles();

    for (size_t i = 0; i < particles.size(); i++) {
        results.push_back(particles[i].pos);
    }
    return results;
}

void ParticlesGD::_bind_methods(){
	ClassDB::bind_method(D_METHOD("spawn", "count", "minBound", "maxBound", "lifeTime", "clr", "radius"), &ParticlesGD::spawn);

	ClassDB::bind_method(D_METHOD("update", "st"), &ParticlesGD::update);

	ClassDB::bind_method(D_METHOD("getParticlePositions"), &ParticlesGD::getParticlePositions);

}
