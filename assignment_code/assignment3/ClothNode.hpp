#ifndef CLOTH_NODE_H_
#define CLOTH_NODE_H_

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

class ClothNode : public SceneNode {
public:
    ClothNode(float integration_step,
              std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
              std::shared_ptr<PendulumSystem> system,
              const ParticleState& initial_state,
              int grid_size)
        : integration_step_(integration_step),
          integrator_(std::move(integrator)),
          system_(system),
          state_(initial_state),
          initial_state_(initial_state),
          time_(0.0f),
          grid_size_(grid_size) {
        
        // Create visual representation
        CreateClothMesh();
    }

    void Update(double delta_time) override {
        // Check for reset key 'R'
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            Reset();
            return;
        }

        // Integrate physics
        float time_remaining = static_cast<float>(delta_time);
        
        while (time_remaining > 0.0f) {
            float step = std::min(time_remaining, integration_step_);
            state_ = integrator_->Integrate(*system_, state_, time_, step);
            time_ += step;
            time_remaining -= step;
        }

        // Update visual representation
        UpdateClothMesh();
    }

private:
    void CreateClothMesh() {
        // Create a node for rendering the cloth as a wireframe
        cloth_node_ = make_unique<SceneNode>();
        
        auto positions = make_unique<PositionArray>();
        auto indices = make_unique<IndexArray>();
        
        // Add all particle positions
        for (const auto& pos : state_.positions) {
            positions->push_back(pos);
        }
        
        // Create line segments for structural springs (horizontal and vertical)
        for (int i = 0; i < grid_size_; i++) {
            for (int j = 0; j < grid_size_; j++) {
                int idx = i * grid_size_ + j;
                
                // Horizontal spring to the right
                if (j < grid_size_ - 1) {
                    indices->push_back(idx);
                    indices->push_back(idx + 1);
                }
                
                // Vertical spring downward
                if (i < grid_size_ - 1) {
                    indices->push_back(idx);
                    indices->push_back(idx + grid_size_);
                }
            }
        }
        
        auto vertex_obj = make_unique<VertexObject>();
        vertex_obj->UpdatePositions(std::move(positions));
        vertex_obj->UpdateIndices(std::move(indices));
        
        auto& rc = cloth_node_->CreateComponent<RenderingComponent>(std::move(vertex_obj));
        rc.SetDrawMode(DrawMode::Lines);
        
        auto shader = std::make_shared<SimpleShader>();
        cloth_node_->CreateComponent<ShadingComponent>(shader);
        
        cloth_node_ptr_ = cloth_node_.get();
        AddChild(std::move(cloth_node_));
    }

    void UpdateClothMesh() {
        auto positions = make_unique<PositionArray>();
        
        // Update all particle positions
        for (const auto& pos : state_.positions) {
            positions->push_back(pos);
        }
        
        auto* rc = cloth_node_ptr_->GetComponentPtr<RenderingComponent>();
        if (rc != nullptr) {
            rc->GetVertexObjectPtr()->UpdatePositions(std::move(positions));
        }
    }

    void Reset() {
        time_ = 0.0f;
        state_ = initial_state_;
    }

    float integration_step_;
    std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
    std::shared_ptr<PendulumSystem> system_;
    ParticleState state_;
    ParticleState initial_state_;
    float time_;
    int grid_size_;
    
    std::unique_ptr<SceneNode> cloth_node_;
    SceneNode* cloth_node_ptr_;
};

}  // namespace GLOO

#endif

