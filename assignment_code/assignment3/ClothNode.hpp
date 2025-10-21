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

// Forward declaration
class SphereNode;

class ClothNode : public SceneNode {
public:
    ClothNode(float integration_step,
              std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
              std::shared_ptr<PendulumSystem> system,
              const ParticleState& initial_state,
              int grid_size,
              SceneNode* collision_sphere = nullptr)
        : integration_step_(integration_step),
          integrator_(std::move(integrator)),
          system_(system),
          state_(initial_state),
          initial_state_(initial_state),
          time_(0.0f),
          grid_size_(grid_size),
          collision_sphere_(collision_sphere) {
        
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
        
        // Handle collisions with sphere - multiple iterations for better resolution
        if (collision_sphere_ != nullptr) {
            // Run collision resolution multiple times to handle deep penetrations
            const int collision_iterations = 3;
            for (int iter = 0; iter < collision_iterations; iter++) {
                HandleSphereCollisions();
            }
        }

        // Update visual representation
        UpdateClothMesh();
    }

private:
    void CreateClothMesh() {
        // Create a node for rendering the cloth with smooth shading
        cloth_node_ = make_unique<SceneNode>();
        
        auto positions = make_unique<PositionArray>();
        auto normals = make_unique<NormalArray>();
        auto indices = make_unique<IndexArray>();
        
        // Add all particle positions
        for (const auto& pos : state_.positions) {
            positions->push_back(pos);
        }
        
        // Compute normals for each vertex
        auto computed_normals = ComputeNormals(state_.positions);
        for (const auto& normal : computed_normals) {
            normals->push_back(normal);
        }
        
        // Create triangles for each quad in the grid
        for (int i = 0; i < grid_size_ - 1; i++) {
            for (int j = 0; j < grid_size_ - 1; j++) {
                int idx = i * grid_size_ + j;
                
                // Each quad is made of two triangles
                // Triangle 1: top-left, bottom-left, top-right
                indices->push_back(idx);                    // top-left
                indices->push_back(idx + grid_size_);       // bottom-left
                indices->push_back(idx + 1);                // top-right
                
                // Triangle 2: top-right, bottom-left, bottom-right
                indices->push_back(idx + 1);                // top-right
                indices->push_back(idx + grid_size_);       // bottom-left
                indices->push_back(idx + grid_size_ + 1);   // bottom-right
            }
        }
        
        auto vertex_obj = make_unique<VertexObject>();
        vertex_obj->UpdatePositions(std::move(positions));
        vertex_obj->UpdateNormals(std::move(normals));
        vertex_obj->UpdateIndices(std::move(indices));
        
        auto& rc = cloth_node_->CreateComponent<RenderingComponent>(std::move(vertex_obj));
        rc.SetDrawMode(DrawMode::Triangles);
        
        // Use Phong shader for smooth shading
        auto shader = std::make_shared<PhongShader>();
        cloth_node_->CreateComponent<ShadingComponent>(shader);
        
        // Add material for the cloth
        auto material = std::make_shared<Material>(
            glm::vec3(0.3f, 0.2f, 0.5f),  // Ambient - purple
            glm::vec3(0.6f, 0.4f, 0.8f),  // Diffuse - light purple
            glm::vec3(1.0f, 1.0f, 1.0f),  // Specular - white
            32.0f                          // Shininess
        );
        cloth_node_->CreateComponent<MaterialComponent>(material);
        
        cloth_node_ptr_ = cloth_node_.get();
        AddChild(std::move(cloth_node_));
    }

    void UpdateClothMesh() {
        auto positions = make_unique<PositionArray>();
        auto normals = make_unique<NormalArray>();
        
        // Update all particle positions
        for (const auto& pos : state_.positions) {
            positions->push_back(pos);
        }
        
        // Recompute normals based on new positions
        auto computed_normals = ComputeNormals(state_.positions);
        for (const auto& normal : computed_normals) {
            normals->push_back(normal);
        }
        
        auto* rc = cloth_node_ptr_->GetComponentPtr<RenderingComponent>();
        if (rc != nullptr) {
            rc->GetVertexObjectPtr()->UpdatePositions(std::move(positions));
            rc->GetVertexObjectPtr()->UpdateNormals(std::move(normals));
        }
    }
    
    // Compute smooth normals for each vertex based on adjacent triangles
    std::vector<glm::vec3> ComputeNormals(const std::vector<glm::vec3>& positions) {
        std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));
        
        // For each quad in the grid, compute face normals and accumulate to vertices
        for (int i = 0; i < grid_size_ - 1; i++) {
            for (int j = 0; j < grid_size_ - 1; j++) {
                int idx = i * grid_size_ + j;
                
                // Get the four corners of this quad
                glm::vec3 p0 = positions[idx];                    // top-left
                glm::vec3 p1 = positions[idx + grid_size_];       // bottom-left
                glm::vec3 p2 = positions[idx + 1];                // top-right
                glm::vec3 p3 = positions[idx + grid_size_ + 1];   // bottom-right
                
                // Triangle 1: p0, p1, p2
                glm::vec3 edge1_a = p1 - p0;
                glm::vec3 edge2_a = p2 - p0;
                glm::vec3 normal_a = glm::cross(edge1_a, edge2_a);
                
                // Accumulate this normal to all three vertices
                normals[idx] += normal_a;
                normals[idx + grid_size_] += normal_a;
                normals[idx + 1] += normal_a;
                
                // Triangle 2: p2, p1, p3
                glm::vec3 edge1_b = p1 - p2;
                glm::vec3 edge2_b = p3 - p2;
                glm::vec3 normal_b = glm::cross(edge1_b, edge2_b);
                
                // Accumulate this normal to all three vertices
                normals[idx + 1] += normal_b;
                normals[idx + grid_size_] += normal_b;
                normals[idx + grid_size_ + 1] += normal_b;
            }
        }
        
        // Normalize all normals
        for (auto& normal : normals) {
            if (glm::length(normal) > 1e-6f) {
                normal = glm::normalize(normal);
            } else {
                // If normal is zero (shouldn't happen), use default up vector
                normal = glm::vec3(0.0f, 0.0f, 1.0f);
            }
        }
        
        return normals;
    }

    void Reset() {
        time_ = 0.0f;
        state_ = initial_state_;
    }
    
    // Handle collisions with a sphere
    void HandleSphereCollisions() {
        // Get sphere properties (using dynamic_cast to check type)
        auto* sphere_node = dynamic_cast<SphereNode*>(collision_sphere_);
        if (sphere_node == nullptr) {
            return;
        }
        
        // Get sphere position in world coordinates
        glm::vec3 sphere_center_world = sphere_node->GetCurrentPosition();
        glm::vec3 sphere_velocity = sphere_node->GetVelocity();
        float sphere_radius = sphere_node->GetRadius();
        
        // Get cloth transform to convert between local and world coordinates
        glm::mat4 cloth_transform = GetTransform().GetLocalToParentMatrix();
        glm::mat4 cloth_inverse = glm::inverse(cloth_transform);
        
        // Convert sphere center to cloth's local coordinate system
        glm::vec3 sphere_center_local = glm::vec3(cloth_inverse * glm::vec4(sphere_center_world, 1.0f));
        
        // Check each particle for collision (particles are in local coordinates)
        for (size_t i = 0; i < state_.positions.size(); i++) {
            glm::vec3& pos = state_.positions[i];
            glm::vec3& vel = state_.velocities[i];
            
            // Calculate vector from sphere center to particle (in local space)
            glm::vec3 to_particle = pos - sphere_center_local;
            float distance = glm::length(to_particle);
            
            // Check if particle is inside or touching the sphere
            // Use consistent epsilon for detection and correction
            const float collision_epsilon = 0.02f;
            if (distance < sphere_radius + collision_epsilon) {
                // Project particle back to sphere surface
                if (distance > 1e-6f) {
                    glm::vec3 normal = to_particle / distance;
                    
                    // Push particle outside sphere surface with same epsilon
                    pos = sphere_center_local + normal * (sphere_radius + collision_epsilon);
                    
                    // Handle velocity - reflect and add sphere velocity
                    // Component of velocity along normal
                    float vel_along_normal = glm::dot(vel, normal);
                    
                    // Only adjust velocity if moving into the sphere
                    if (vel_along_normal < 0.0f) {
                        // Remove the component going into the sphere
                        vel -= vel_along_normal * normal;
                        
                        // Add sphere's velocity component (frictionless collision)
                        // Convert sphere velocity to local space
                        glm::vec3 sphere_vel_local = glm::vec3(cloth_inverse * glm::vec4(sphere_velocity, 0.0f));
                        float sphere_vel_along_normal = glm::dot(sphere_vel_local, normal);
                        vel += sphere_vel_along_normal * normal;
                        
                        // Reduced damping to prevent stickiness (was 0.7)
                        vel *= 0.9f;
                    }
                } else {
                    // Particle exactly at center - push it out in arbitrary direction
                    const float collision_epsilon = 0.02f;
                    pos = sphere_center_local + glm::vec3(sphere_radius + collision_epsilon, 0.0f, 0.0f);
                    vel *= 0.5f;  // Reduce velocity significantly
                }
            }
        }
    }

    float integration_step_;
    std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
    std::shared_ptr<PendulumSystem> system_;
    ParticleState state_;
    ParticleState initial_state_;
    float time_;
    int grid_size_;
    SceneNode* collision_sphere_;  // Non-owning pointer to sphere for collision
    
    std::unique_ptr<SceneNode> cloth_node_;
    SceneNode* cloth_node_ptr_;
};

}  // namespace GLOO

#endif

