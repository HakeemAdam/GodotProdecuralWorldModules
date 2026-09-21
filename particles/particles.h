#pragma once
#include <algorithm>
#include <random>
#include "core/math/color.h"
#include "core/math/vector3.h"
#include "../utilities/treeops.h"
#include <vector>

struct Particle{
	Vector3 pos;
	Vector3 vel;
	Color color;
	float age;
	float radius;
};

class ParticleSystem{
	private:
		std::vector<Particle> particles;
		Vector3 minBbox = {0,0,0};
		Vector3 maxBbox = {0,0,0};
		int particleCount = 0;
		float max_radius = 0.0f;
		std::vector<Vector3> positions;

		// Octree
		OctNode* tree = nullptr;

		AABB3 makeBounds(){
			Vector3 center = (minBbox + maxBbox) * 0.5f;
			Vector3 span = maxBbox - minBbox;
			float halfSize = std::max({span.x, span.y, span.z}) * 0.5f;
			return AABB3{center, halfSize};
		}

		float randFloat_range(float min, float max){
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_real_distribution<float> rng(min, max);
			return rng(gen);
		}

		// Particle helpers
		void keepInBounds() {
			for (auto& p : particles) {
				if (p.pos.x < minBbox.x) { p.pos.x = minBbox.x; p.vel.x *= -1; }
				if (p.pos.x > maxBbox.x) { p.pos.x = maxBbox.x; p.vel.x *= -1; }
				if (p.pos.y < minBbox.y) { p.pos.y = minBbox.y; p.vel.y *= -1; }
				if (p.pos.y > maxBbox.y) { p.pos.y = maxBbox.y; p.vel.y *= -1; }
				if (p.pos.z < minBbox.z) { p.pos.z = minBbox.z; p.vel.z *= -1; }
				if (p.pos.z > maxBbox.z) { p.pos.z = maxBbox.z; p.vel.z *= -1; }
			}
		}



	public:
		ParticleSystem() = default;
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;

        ~ParticleSystem(){freeOctree(tree);}

		static constexpr float INFINITE_LIFETIME = 1.0f;
		void spawn(int count, Vector3 minBound, Vector3 maxBound, float lifeTime, Color clr, float radius = 2.0f){

			particles.clear();
			positions.clear();

			minBbox = minBound;
			maxBbox = maxBound;
			particleCount = count;

			for (int i = 0; i < particleCount; i++){
				Particle p;
				p.pos = {
					randFloat_range(minBbox.x, maxBbox.x),
					randFloat_range(minBbox.y, maxBbox.y),
					randFloat_range(minBbox.z, maxBbox.z)
				};
				p.age = lifeTime;
				p.color = clr;
				p.radius = radius;

				particles.push_back(p);
				positions.push_back(p.pos);
			}


			freeOctree(tree);
			AABB3 rootBounds = makeBounds();
			tree = buildOctree(positions, rootBounds);
			randomVelocity();
		};

		void randomVelocity(float minSpeed = 0.5f, float maxSpeed = 1.0f){
			constexpr float TWO_PI = 6.28318530718f;
			for (auto& p : particles){
				float theta = randFloat_range(0.0f, TWO_PI);
				float z = randFloat_range(-1.0f, 1.0f);
				float r = std::sqrt(std::max(0.0f, 1.0f -z * z));
				float speed = randFloat_range(minSpeed, maxSpeed);


				p.vel = {
					r * std::cos(theta) * speed,
					r * std::sin(theta) * speed,
					z * speed
				};
			}
		};

		void update(float dt){

			applyForce(dt);
			for (auto& p: particles){
				p.pos += p.vel * dt;
				if ( p.age != INFINITE_LIFETIME){
					p.age -= dt;
				}
			}
			keepInBounds();
			removeDead();
			rebuildTree();
			octreeCollision();

		};


		void applyForce(float dt){
			static const Vector3 gravity = {0.0f, -9.8f, 0.0f};
			const float dragCoeff = 0.15f;

			for (auto& p : particles){
				Vector3 force = gravity;
				p.vel += force * dt;
				p.vel -= p.vel * (dragCoeff * dt);
			}

		};

		void rebuildTree(){
			freeOctree(tree);
			positions.clear();
			positions.reserve(particles.size());
			max_radius = 0.0f;
			for (auto& p: particles){
				positions.push_back(p.pos);
				max_radius = std::max(max_radius, p.radius);
			}

			tree = positions.empty() ? nullptr : buildOctree(positions, makeBounds());
		}

		void removeDead(){
			particles.erase(
					std::remove_if(particles.begin(), particles.end(),
						[](const Particle& p){return p.age <= 0.0f;}),
					particles.end()
					);
		}

		void octreeCollision(int solverIterations = 1){
			if (!tree) {return;}
			const float restitution = 0.5f;

			for (int iter = 0; iter < solverIterations; iter++){
				for(int i =0; i < particles.size(); i++){
					float radius = particles[i].radius;
					Vector3 queryPos = particles[i].pos;

					std::vector<int> neighbors;
					queryRadius(tree, queryPos, radius * max_radius, positions, neighbors);

					for (int j : neighbors){
						if (static_cast<size_t>(j) <= i) {continue;}

						float radius_j = particles[j].radius;
						float minDist = radius + radius_j;

						Vector3 disp = particles[i].pos - particles[j].pos;
						float dist = disp.length();

						if ( dist < minDist && dist > 0.00001f){

							Vector3 normal = disp / dist;
							float overlap = minDist - dist;

							particles[i].pos += normal * (overlap * 0.5f);
							particles[j].pos += normal * (overlap * 0.5f);

							Vector3 relVel = particles[i].vel - particles[j].vel;
							float vRel = relVel.x * normal.x + relVel.y * normal.y + relVel.z * normal.z;

							if (vRel < 0.0f) {
								float impulse = -(1.0f + restitution) * vRel * 0.5f; particles[i].vel += normal * impulse;
								particles[j].vel += normal * impulse;
							}

						}

					}


				}
			}
		}

		const std::vector<Particle>& getParticles () const { return particles; }
		size_t getParticleCount() const { return particles.size();}



};
