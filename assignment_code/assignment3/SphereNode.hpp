#ifndef SPHERE_NODE_H_
#define SPHERE_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"

namespace GLOO {

class SphereNode : public SceneNode {
public:
    SphereNode(float radius, const glm::vec3& initial_position)
        : radius_(radius),
          time_(0.0f),
          center_offset_(initial_position) {
        
        // Create sphere mesh
        auto sphere_mesh = PrimitiveFactory::CreateSphere(radius, 25, 25);
        CreateComponent<RenderingComponent>(std::move(sphere_mesh));
        
        // Add shader
        auto shader = std::make_shared<PhongShader>();
        CreateComponent<ShadingComponent>(shader);
        
        // Add material (orange/yellow color)
        auto material = std::make_shared<Material>(
            glm::vec3(0.8f, 0.4f, 0.1f),  // Ambient - orange
            glm::vec3(1.0f, 0.6f, 0.2f),  // Diffuse - bright orange
            glm::vec3(1.0f, 1.0f, 1.0f),  // Specular - white
            64.0f                          // Shininess
        );
        CreateComponent<MaterialComponent>(material);
        
        // Set initial position
        GetTransform().SetPosition(GetCurrentPosition());
    }
    
    void Update(double delta_time) override {
        time_ += static_cast<float>(delta_time);
        
        // Update sphere position
        GetTransform().SetPosition(GetCurrentPosition());
    }
    
    // Get the current center position of the sphere
    glm::vec3 GetCurrentPosition() const {
        // Move perpendicular to cloth plane (in Z direction)
        // The cloth is in XY plane, so sphere moves back and forth in Z
        // Increased amplitude and adjusted offset so sphere passes through cloth plane
        float amplitude = 1.8f;  // Increased from 1.2f
        float frequency = 0.6f;
        
        return center_offset_ + glm::vec3(
            0.0f,                                           // No X movement
            0.0f,                                           // No Y movement  
            amplitude * std::sin(frequency * time_)        // Z oscillation (perpendicular to cloth)
        );
    }
    
    // Get the velocity of the sphere
    glm::vec3 GetVelocity() const {
        float amplitude = 1.8f;  // Match position amplitude
        float frequency = 0.6f;
        
        return glm::vec3(
            0.0f,                                           // No X velocity
            0.0f,                                           // No Y velocity
            amplitude * frequency * std::cos(frequency * time_)  // Z velocity
        );
    }
    
    float GetRadius() const {
        return radius_;
    }

private:
    float radius_;
    float time_;
    glm::vec3 center_offset_;
};

}  // namespace GLOO

#endif

