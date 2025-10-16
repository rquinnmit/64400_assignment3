#ifndef TRAPEZOIDAL_INTEGRATOR_H_
#define TRAPEZOIDAL_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class TrapezoidalIntegrator : public IntegratorBase<TSystem, TState> {
    TState Integrate(const TSystem& system,
                    const TState& state, 
                    float start_time,
                    float dt) const override {
        // Trapezoidal Rule
        TState f0 = system.ComputeTimeDerivative(state, start_time);
        TState temp_state = state + f0 * dt;
        TState f1 = system.ComputeTimeDerivative(temp_state, start_time + dt);
        return state + (f0 + f1) * (dt / 2.0f);
    }
};
} // namespace GLOO

#endif