#include "SimulationApp.hpp"

#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"

#include "IntegratorFactory.hpp"
#include "SimpleCircularNode.hpp"
#include "PendulumNode.hpp"
#include "ClothNode.hpp"


namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             IntegratorType integrator_type,
                             float integration_step)
    : Application(app_name, window_size),
      integrator_type_(integrator_type),
      integration_step_(integration_step) {
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 10.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.2f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
  root.AddChild(std::move(point_light_node));

  // ========== Example 1: Simple Circular Motion (Left) ==========
  {
    auto integrator = IntegratorFactory::CreateIntegrator<SimpleCircularSystem, ParticleState>(
        integrator_type_);
    auto simple_node = make_unique<SimpleCircularNode>(
        integration_step_, std::move(integrator));
    simple_node->GetTransform().SetPosition(glm::vec3(-3.0f, 0.0f, 0.0f));
    root.AddChild(std::move(simple_node));
  }

  // ========== Example 2: Pendulum Chain (Middle) ==========
  {
    // Create pendulum system with 4 particles
    auto system = std::make_shared<PendulumSystem>();
    
    // Set physics parameters
    system->SetGravity(glm::vec3(0.0f, -9.8f, 0.0f));
    system->SetDragCoefficient(0.5f);  // Adjust for stability
    
    // Add 4 particles in a vertical chain
    const int num_particles = 4;
    const float particle_mass = 1.0f;
    const float spring_stiffness = 100.0f;
    const float spring_rest_length = 0.5f;
    
    for (int i = 0; i < num_particles; i++) {
      system->AddParticle(particle_mass, false);
    }
    
    // Fix the first particle (top of chain)
    system->SetParticleFixed(0, true);
    
    // Connect particles with springs to form a chain
    for (int i = 0; i < num_particles - 1; i++) {
      system->AddSpring(i, i + 1, spring_stiffness, spring_rest_length);
    }
    
    // Initialize particle positions (vertical chain)
    ParticleState initial_state;
    initial_state.positions.resize(num_particles);
    initial_state.velocities.resize(num_particles);
    
    for (int i = 0; i < num_particles; i++) {
      initial_state.positions[i] = glm::vec3(0.0f, -i * spring_rest_length, 0.0f);
      initial_state.velocities[i] = glm::vec3(0.0f);
    }
    
    // Create integrator and node
    auto integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(
        integrator_type_);
    auto pendulum_node = make_unique<PendulumNode>(
        integration_step_, std::move(integrator), system, initial_state);
    pendulum_node->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
    root.AddChild(std::move(pendulum_node));
  }

  // ========== Example 3: Cloth (Right) ==========
  {
    // Create cloth system with 8x8 grid of particles
    auto system = std::make_shared<PendulumSystem>();
    
    // Set physics parameters
    system->SetGravity(glm::vec3(0.0f, -9.8f, 0.0f));
    system->SetDragCoefficient(2.0f);  // Higher drag for cloth stability
    
    const int grid_size = 8;
    const float particle_mass = 0.5f;
    const float spacing = 0.25f;  // Distance between adjacent particles
    
    // Spring stiffness values
    const float structural_stiffness = 80.0f;
    const float shear_stiffness = 40.0f;
    const float flex_stiffness = 40.0f;
    
    // Helper function to get particle index from grid coordinates
    auto IndexOf = [grid_size](int i, int j) -> int {
      return i * grid_size + j;
    };
    
    // Add all particles in a grid
    for (int i = 0; i < grid_size; i++) {
      for (int j = 0; j < grid_size; j++) {
        system->AddParticle(particle_mass, false);
      }
    }
    
    // Fix the top two corners
    system->SetParticleFixed(IndexOf(0, 0), true);
    system->SetParticleFixed(IndexOf(0, grid_size - 1), true);
    
    // Add structural springs (horizontal and vertical neighbors)
    for (int i = 0; i < grid_size; i++) {
      for (int j = 0; j < grid_size; j++) {
        // Horizontal spring to the right
        if (j < grid_size - 1) {
          system->AddSpring(IndexOf(i, j), IndexOf(i, j + 1), 
                           structural_stiffness, spacing);
        }
        
        // Vertical spring downward
        if (i < grid_size - 1) {
          system->AddSpring(IndexOf(i, j), IndexOf(i + 1, j), 
                           structural_stiffness, spacing);
        }
      }
    }
    
    // Add shear springs (diagonal neighbors)
    for (int i = 0; i < grid_size - 1; i++) {
      for (int j = 0; j < grid_size - 1; j++) {
        // Diagonal down-right
        system->AddSpring(IndexOf(i, j), IndexOf(i + 1, j + 1), 
                         shear_stiffness, spacing * std::sqrt(2.0f));
        
        // Diagonal down-left
        system->AddSpring(IndexOf(i, j + 1), IndexOf(i + 1, j), 
                         shear_stiffness, spacing * std::sqrt(2.0f));
      }
    }
    
    // Add flex springs (skip one particle - 2 units away)
    for (int i = 0; i < grid_size; i++) {
      for (int j = 0; j < grid_size; j++) {
        // Horizontal flex spring (2 units right)
        if (j < grid_size - 2) {
          system->AddSpring(IndexOf(i, j), IndexOf(i, j + 2), 
                           flex_stiffness, spacing * 2.0f);
        }
        
        // Vertical flex spring (2 units down)
        if (i < grid_size - 2) {
          system->AddSpring(IndexOf(i, j), IndexOf(i + 2, j), 
                           flex_stiffness, spacing * 2.0f);
        }
      }
    }
    
    // Initialize particle positions in a grid
    ParticleState initial_state;
    initial_state.positions.resize(grid_size * grid_size);
    initial_state.velocities.resize(grid_size * grid_size);
    
    for (int i = 0; i < grid_size; i++) {
      for (int j = 0; j < grid_size; j++) {
        int idx = IndexOf(i, j);
        initial_state.positions[idx] = glm::vec3(
          j * spacing - (grid_size - 1) * spacing / 2.0f,  // Center horizontally
          -i * spacing,                                      // Hang downward
          0.0f
        );
        initial_state.velocities[idx] = glm::vec3(0.0f);
      }
    }
    
    // Create integrator and node
    auto integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(
        integrator_type_);
    auto cloth_node = make_unique<ClothNode>(
        integration_step_, std::move(integrator), system, initial_state, grid_size);
    cloth_node->GetTransform().SetPosition(glm::vec3(3.0f, 2.0f, 0.0f));
    root.AddChild(std::move(cloth_node));
  }
}
}  // namespace GLOO