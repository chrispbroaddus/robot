
#pragma once

namespace imu_propagator {

/// Runge Kutta integrator for solving Ordinary Differential Equations
template <typename SCALAR, typename ODE> class RungeKutta4thOrderIntegrator {
public:
    using y_type = typename ODE::y_type;

    /// Integrate the Ordinary Differential Equation (ODE) with initial conditions.
    /// \param ode reference to the ODE
    /// \param step step to evaluate
    /// \return solution to the ODE
    y_type integrate(const ODE& ode, const SCALAR step) {
        const y_type k1 = ode.evaluate(ode.t0(), ode.initialValue(), step);
        const y_type k2 = ode.evaluate(ode.t0() + step / 2, ode.initialValue() + k1 / 2, step);
        const y_type k3 = ode.evaluate(ode.t0() + step / 2, ode.initialValue() + k2 / 2, step);
        const y_type k4 = ode.evaluate(ode.t0() + step, ode.initialValue() + k3, step);
        return ode.initialValue() + (k1 + 2 * k2 + 2 * k3 + k4) * (step / 6);
    }
};
}