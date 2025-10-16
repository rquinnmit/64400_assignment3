#ifndef SIMPLE_CIRCULAR_SYSTEM_H_
#define SIMPLE_CIRCULAR_SYSTEM_H_

#include "ParticleSystemBase.hpp"

namespace GLOO {
class SimpleCircularSystem : public ParticleSystemBase {
    public:
        ParticleState ComputeTimeDerivative(const ParticleState& state,
                                            float time) const override {
            // ParticleState with a single particle
            ParticleState derivative;
            derivative.positions.resize(1);
            derivative.velocities.resize(1);

            const glm::vec3& pos = state.positions[0];
            derivative.positions[0] = glm::vec3(-pos.y, pos.x, 0.0f);
            derivative.velocities[0] = glm::vec3(0.0f);

            return derivative;
        }
    };
} // namespace GLOO


#endif
