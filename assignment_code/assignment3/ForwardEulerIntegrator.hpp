#ifndef FORWARD_EULER_INTEGRATOR_H_
#define FORWARD_EULER_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardEulerIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    // Forward Euler
    TState f0 = system.ComputeTimeDerivative(state, start_time);
    return state + f0 * dt;
  }
};
}  // namespace GLOO

#endif
