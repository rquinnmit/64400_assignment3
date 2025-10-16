#ifndef SIMPLE_CIRCULAR_NODE_H_
#define SIMPLE_CIRCULAR_NODE_H_

#include "gloo/SceneNode.hpp"
#include "IntegratorBase.hpp"
#include "ParticleState.hpp"
#include "SimpleCircularSystem.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

namespace GLOO {
class SimpleCircularNode : public SceneNode {
    public:
        SimpleCircularNode(float integration_step,
                           std::unique_ptr<IntegratorBase<SimpleCircularSystem, ParticleState>> integrator)
                            : integration_step_(integration_step),
                            integrator_(std::move(integrator)),
                            time_(0.0f) {
                // Initialize state with single particle
                state_.positions.resize(1);
                state_.velocities.resize(1);
                state_.positions[0] = glm::vec3(1.0f, 0.0f, 0.0f);
                state_.velocities[0] = glm::vec3(0.0f);

                auto sphere_mesh = PrimitiveFactory::CreateSphere(0.05f, 10, 10);
                CreateComponent<RenderingComponent>(std::move(sphere_mesh));
                
                auto shader = std::make_shared<PhongShader>();
                CreateComponent<ShadingComponent>(shader);
                
                auto material = std::make_shared<Material>(
                    glm::vec3(1.0f, 0.0f, 0.0f),  // Ambient
                    glm::vec3(1.0f, 0.0f, 0.0f),  // Diffuse (red)
                    glm::vec3(1.0f, 1.0f, 1.0f),  // Specular (white)
                    32.0f                          // Shininess
                );
                CreateComponent<MaterialComponent>(material);

                GetTransform().SetPosition(state_.positions[0]);
            }
        
        void Update(double delta_time) override {
            float time_remaining = static_cast<float>(delta_time);

            while (time_remaining > 0.0f) {
                float step = std::min(time_remaining, integration_step_);
                state_ = integrator_->Integrate(system_, state_, time_, step);
                time_ += step;
                time_remaining -= step;
            }

            GetTransform().SetPosition(state_.positions[0]);
        }

    private:
        float integration_step_;
        std::unique_ptr<IntegratorBase<SimpleCircularSystem, ParticleState>> integrator_;
        SimpleCircularSystem system_;
        ParticleState state_;
        float time_;
};
} // namespace GLOO

#endif
