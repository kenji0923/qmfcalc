#include <qmscalc/voltage_calculator.h>

#include <cmath>

#include <qmscalc/resolution_estimator.h>


namespace qmscalc {


double get_oscillation_energy(const double m, const double omega, const double r0)
{
    return m * std::pow(omega * r0, 2);
}


double get_dc_voltage(const double m, const double omega, const double r0, const double a)
{
    return a * get_oscillation_energy(m, omega, r0) / 4 / kElementaryCharge;
}


double get_rf_voltage(const double m, const double omega, const double r0, const double q)
{
    return q * get_oscillation_energy(m, omega, r0) / 2 / kElementaryCharge;
}


VoltageSolution compute_target_voltages(const double m, const double delta_m, const double r0,
					const double f)
{
    VoltageSolution solution{};
    solution.resolution = m / delta_m;

    const double slope = find_slope_for_mass_resolution(solution.resolution);
    if (slope < 0) {
	solution.transmitted = false;
	return solution;
    }

    const ResolutionEstimate est = estimate_mass_resolution_from_slope(slope);
    const double omega = 2 * M_PI * f;

    solution.transmitted = est.transmitted;
    solution.slope = est.slope;
    solution.q = est.q_center;
    solution.a = est.a_center;
    solution.U = get_dc_voltage(m, omega, r0, est.a_center);
    solution.V = get_rf_voltage(m, omega, r0, est.q_center);

    return solution;
}


} // namespace qmscalc
