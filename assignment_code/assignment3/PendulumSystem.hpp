#ifndef PENDULUM_SYSTEM_H_
#define PENDULUM_SYSTEM_H_

#include "ParticleSystemBase.hpp"
#include <vector>

namespace GLOO {

struct Spring {
    int particle1_index;
    int particle2_index;
    float stiffness;
    float rest_length;

    Spring(int p1, int p2, float k, float l)
        : particle1_index(p1), particle2_index(p2), stiffness(k), rest_length(l) {}
};

struct Particle {
    float mass;
    bool fixed; // particle doesn't move if true

    Particle(float m = 1.0f, bool is_fixed = false)
        : mass(m), fixed(is_fixed) {}
};

class PendulumSystem : public ParticleSystemBase {
public:
    PendulumSystem()
        : gravity_(glm::vec3(0.0f, -9.8f, 0.0f)),
          drag_coefficient_(0.5f) {}

    int AddParticle(float mass, bool fixed = false) {
        particles_.push_back(Particle(mass, fixed));
        return static_cast<int>(particles_.size() - 1);
    }

    void AddSpring(int particle1_index, int particle2_index, float stiffness, float rest_length) {
        springs_.push_back(Spring(particle1_index, particle2_index, stiffness, rest_length));
    }

    void SetParticleFixed(int index, bool fixed) {
        if (index >= 0 && index < static_cast<int>(particles_.size())) {
            particles_[index].fixed = fixed;
        }
    }

    void SetGravity(const glm::vec3& g) {
        gravity_ = g;
    }

    void SetDragCoefficient(float k) {
        drag_coefficient_ = k;
    }

    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override {
        ParticleState derivative;
        int num_particles = static_cast<int>(state.positions.size());
        derivative.positions.resize(num_particles);
        derivative.velocities.resize(num_particles);
        
        for (int i = 0; i < num_particles; i++) {
            if (particles_[i].fixed) {
                derivative.positions[i] = glm::vec3(0.0f);
                derivative.velocities[i] = glm::vec3(0.0f);
            } else {
                derivative.positions[i] = state.velocities[i];
                glm::vec3 total_force(0.0f);
                total_force += particles_[i].mass * gravity_;
                total_force += -drag_coefficient_ * state.velocities[i];

                for (const auto& spring : springs_) {
                    int other_index = -1;

                    if (spring.particle1_index == i) {
                        other_index = spring.particle2_index;
                    } else if (spring.particle2_index == i) {
                        other_index = spring.particle1_index;
                    }

                    if (other_index != -1) {
                        glm::vec3 d = state.positions[i] - state.positions[other_index];
                        float length = glm::length(d);

                        if (length > 1e-6f) {
                            glm::vec3 direction = d / length;
                            float displacement = length - spring.rest_length;
                            glm::vec3 spring_force = -spring.stiffness * displacement * direction;
                            total_force += spring_force;
                        }
                    }
                }

                derivative.velocities[i] = total_force / particles_[i].mass;
            }
        }

        return derivative;
    }

    size_t GetNumParticles() const {
        return particles_.size();
    }

    const std::vector<Spring>& GetSprings() const {
        return springs_;
    }

private:
    std::vector<Particle> particles_;
    std::vector<Spring> springs_;
    glm::vec3 gravity_;
    float drag_coefficient_;
};
} // namespace GLOO

#endif
