#ifndef PENDULUM_NODE_H_
#define PENDULUM_NODE_H_

#include "gloo/SceneNode.hpp"
#include "IntegratorBase.hpp"
#include "ParticleState.hpp"
#include "PendulumSystem.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {

class PendulumNode : public SceneNode {
public:
    PendulumNode(float integration_step,
                std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
                std::shared_ptr<PendulumSystem> system,
                const ParticleState& initial_state)
        : integration_step_(integration_step),
          integrator_(std::move(integrator)),
          system_(system),
          state_(initial_state),
          time_(0.0f) {
        
        CreateParticleSphere();
        CreateSpringLines();
    }

    void Update(double delta_time) override {
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            Reset();
            return;
        }

        float time_remaining = static_cast<float>(delta_time);
        while (time_remaining > 0.0f) {
            float step = std::min(time_remaining, integration_step_);
            state_ = integrator_->Integrate(*system_, state_, time_, step);
            time_ += step;
            time_remaining -= step;
        }

        UpdateParticleSpheres();
        UpdateSpringLines();
    }

private:
    void CreateParticleSphere() {
        size_t num_particles = system_->GetNumParticles();

        for (size_t i = 0; i < num_particles; i++) {
            auto sphere_node = make_unique<SceneNode>();
            
            auto sphere_mesh = PrimitiveFactory::CreateSphere(0.08f, 10, 10);
            sphere_node->CreateComponent<RenderingComponent>(std::move(sphere_mesh));

            auto shader = std::make_shared<PhongShader>();
            sphere_node->CreateComponent<ShadingComponent>(shader);

            auto material= std::make_shared<Material>(
                glm::vec3(0.0f, 0.2f, 0.6f),
                glm::vec3(0.0f, 0.4f, 0.8f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                32.0f
            );
            sphere_node->CreateComponent<MaterialComponent>(material);
            
            sphere_node->GetTransform().SetPosition(state_.positions[i]);
            particle_nodes_.push_back(sphere_node.get());
            AddChild(std::move(sphere_node));
        }
    }

    void CreateSpringLines() {
        spring_node_ = make_unique<SceneNode>();

        auto positions = make_unique<PositionArray>();
        auto indices = make_unique<IndexArray>();

        const auto& springs = system_->GetSprings();
        for (size_t i = 0; i < springs.size(); i++) {
            const auto& spring = springs[i];

            positions->push_back(state_.positions[spring.particle1_index]);
            positions->push_back(state_.positions[spring.particle2_index]);
            indices->push_back(static_cast<unsigned int>(i * 2));
            indices->push_back(static_cast<unsigned int>(i * 2 + 1));
        }

        auto vertex_obj = make_unique<VertexObject>();
        vertex_obj->UpdatePositions(std::move(positions));
        vertex_obj->UpdateIndices(std::move(indices));
        
        auto& rc = spring_node_->CreateComponent<RenderingComponent>(std::move(vertex_obj));
        rc.SetDrawMode(DrawMode::Lines);
        
        auto shader = std::make_shared<SimpleShader>();
        spring_node_->CreateComponent<ShadingComponent>(shader);
        
        spring_node_ptr_ = spring_node_.get();
        AddChild(std::move(spring_node_));
    }

    void UpdateParticleSpheres() {
        for (size_t i = 0; i < particle_nodes_.size(); i++) {
            particle_nodes_[i]->GetTransform().SetPosition(state_.positions[i]);
        }
    }

    void UpdateSpringLines() {
        auto positions = make_unique<PositionArray>();
        
        const auto& springs = system_->GetSprings();
        for (const auto& spring : springs) {
            positions->push_back(state_.positions[spring.particle1_index]);
            positions->push_back(state_.positions[spring.particle2_index]);
        }
        
        auto* rc = spring_node_ptr_->GetComponentPtr<RenderingComponent>();
        if (rc != nullptr) {
            rc->GetVertexObjectPtr()->UpdatePositions(std::move(positions));
        }
    }

    void Reset() {
        // Reset to initial state (would need to store initial_state_ as member)
        time_ = 0.0f;
        // For now, just reset velocities to zero
        for (auto& vel : state_.velocities) {
            vel = glm::vec3(0.0f);
        }
    }

    float integration_step_;
    std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
    std::shared_ptr<PendulumSystem> system_;
    ParticleState state_;
    float time_;
    
    std::vector<SceneNode*> particle_nodes_;  // Non-owning pointers to particle spheres
    std::unique_ptr<SceneNode> spring_node_;
    SceneNode* spring_node_ptr_;  // Non-owning pointer for updates
};
} // namespace GLOO

#endif
