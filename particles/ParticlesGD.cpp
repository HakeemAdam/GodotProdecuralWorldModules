#include "ParticlesGD.h"
#include "core/object/class_db.h"
#include "core/object/property_info.h"
#include "core/variant/variant.h"
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

Vector3 ParticlesGD::get_gravity(){
	return p_system->gravity;
}

void ParticlesGD::set_gravity(Vector3 p_gravity){
	if (!p_system) {
		return;
	}
	p_system->gravity =p_gravity;
}

float ParticlesGD::get_drag_coeff() const{
	return p_system ? p_system->dragCoeff : 0.0f;
}

void ParticlesGD::set_drag_coeff(float p_value){
	if (p_system){
		p_system->dragCoeff = p_value;
	}
}

float ParticlesGD::get_separation_dist() const{
	return p_system ? p_system->separationDist : 0.0f;
}

void ParticlesGD::set_separtion_dist(float p_value){
	if (p_system){
		p_system->separationDist = p_value;
	}
}

float ParticlesGD::get_separation_weight() const{
	return p_system ? p_system->separationWeight : 0.0f;
}

void ParticlesGD::set_separation_weight(float p_value){
	if (p_system){
		p_system->separationWeight = p_value;
	}
}

float ParticlesGD::get_cohesion_weight() const{
	return p_system ? p_system->cohesionWeight : 0.0f;
}

void ParticlesGD::set_cohesion_weight(float p_value){
	if (p_system){
		p_system->cohesionWeight = p_value;
	}
}

float ParticlesGD::get_alignment_weight() const{
	return p_system ? p_system->alignmentWeight : 0.0f;
}

void ParticlesGD::set_alignment_weight(float p_value){
	if (p_system){
		p_system->alignmentWeight = p_value;
	}
}

float ParticlesGD::get_restitution() const{
	return p_system ? p_system->restitution : 0.0f;
}

void ParticlesGD::set_restitution(float p_value){
	if (p_system){
		p_system->restitution = p_value;
	}
}

float ParticlesGD::get_max_speed() const{
	return p_system ? p_system->max_speed : 0.0f;
}

void ParticlesGD::set_max_speed(float p_value){
	if (p_system){
		p_system->max_speed = p_value;
	}
}

float ParticlesGD::get_max_force() const{
	return p_system ? p_system->max_force : 0.0f;
}

void ParticlesGD::set_max_force(float p_value){
	if (p_system){
		p_system->max_force = p_value;
	}
}


void ParticlesGD::_bind_methods(){
	ClassDB::bind_method(D_METHOD("spawn", "count", "minBound", "maxBound", "lifeTime", "clr", "radius"), &ParticlesGD::spawn);

	ClassDB::bind_method(D_METHOD("update", "st"), &ParticlesGD::update);

	ClassDB::bind_method(D_METHOD("getParticlePositions"), &ParticlesGD::getParticlePositions);

	ClassDB::bind_method(D_METHOD("get_gravity"), &ParticlesGD::get_gravity);

	ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &ParticlesGD::set_gravity);

	ClassDB::bind_method(D_METHOD("get_drag_coeff"), &ParticlesGD::get_drag_coeff);

	ClassDB::bind_method(D_METHOD("set_drag_coeff", "p_value"), &ParticlesGD::set_drag_coeff);

	ClassDB::bind_method(D_METHOD("get_separation_dist"), &ParticlesGD::get_separation_dist);

	ClassDB::bind_method(D_METHOD("set_separation_dist", "p_value"), &ParticlesGD::set_separtion_dist);

	ClassDB::bind_method(D_METHOD("get_separation_weight"), &ParticlesGD::get_separation_weight);

	ClassDB::bind_method(D_METHOD("set_separation_weight", "p_value"), &ParticlesGD::set_separation_weight);

	ClassDB::bind_method(D_METHOD("get_cohesion_weight"), &ParticlesGD::get_cohesion_weight);

	ClassDB::bind_method(D_METHOD("set_cohesion_weight", "p_value"), &ParticlesGD::set_cohesion_weight);

	ClassDB::bind_method(D_METHOD("get_alignment_weight"), &ParticlesGD::get_alignment_weight);

	ClassDB::bind_method(D_METHOD("set_alignment_weight", "p_value"), &ParticlesGD::set_alignment_weight);

	ClassDB::bind_method(D_METHOD("get_restitution"), &ParticlesGD::get_restitution);

	ClassDB::bind_method(D_METHOD("set_restitution", "p_value"), &ParticlesGD::set_restitution);

	ClassDB::bind_method(D_METHOD("get_max_speed"), &ParticlesGD::get_max_speed);

	ClassDB::bind_method(D_METHOD("set_max_speed", "p_value"), &ParticlesGD::set_max_speed);

	ClassDB::bind_method(D_METHOD("get_max_force"), &ParticlesGD::get_max_force);

	ClassDB::bind_method(D_METHOD("set_max_force", "p_value"), &ParticlesGD::set_max_force);

	// TODO:Remove
	/*
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "gravity"), "set_gravity", "get_gravity");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "drag_coeff"), "set_drag_coeff", "get_drag_coeff");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "separation_dist"), "set_separation_dist", "get_separation_dist");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "separation_weight"), "set_separation_weight", "get_separation_weight");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cohesion_weight"), "set_cohesion_weight", "get_cohesion_weight");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "alignment_weight"), "set_alignment_weight", "get_alignment_weight");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "restitution"), "set_restitution", "get_restitution");
*/
}
