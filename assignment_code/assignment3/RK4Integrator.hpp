#ifndef RK4_INTEGRATOR_H_
#define RK4_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class RK4Integrator : public IntegratorBase<TSystem, TState> {
    TState Integrate(const TSystem& system,
                    const TState& state,
                    float start_time,
                    float dt) const override {
        // RK4
        TState k1 = system.ComputeTimeDerivative(state, start_time);
        TState k2 = system.ComputeTimeDerivative(state + k1 * (dt / 2.0f), start_time + dt / 2.0f);
        TState k3 = system.ComputeTimeDerivative(state + k2 * (dt / 2.0f), start_time + dt / 2.0f);
        TState k4 = system.ComputeTimeDerivative(state + k3 * dt, start_time + dt);

        return state + (k1 + k2 * 2.0f + k3 * 2.0f + k4) * (dt / 6.0f);
    }
};
} // namespace GLOO

#endif