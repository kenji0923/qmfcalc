#include <qmfcalc/matrix_method_calculator.h>

#include <cmath>
#include <vector>

#include <qmfcalc/result.h>


namespace qmfcalc {


double MatrixMethodCalculator::get_u_max_infinite_length(
	const double mass,
	const IonMotion& ion_motion,
	const double rod_length
    ) const
{
    const double flight_time = rod_length / ion_motion.v_z;

    double u = ion_motion.x;
    double v = ion_motion.v_x;

    PhaseSpacePoint p_x(RF_omega_, u, v);
    evolve(p_x, flight_time);
}


std::vector<Result::Efficiency> MatrixMethodCalculator::get_efficiency_mass_spectrum_infinite_length(
	const std::array<double, 2>& mass_range,
	const double mass_step,
	const std::vector<IonMotion>& ion_motions,
	const RodConfig& rod_config
    ) const
{
    std::vector<Result::Efficiency> resutls;

    for (double m = mass_range[0]; m < mass_range[1]; m += mass_step) {
	double transmitted = 0;

	for (const auto& ion_motion : ion_motions) {
	    const double u_max = get_u_max_infinite_length(m, ion_motion);

	    if (u_max < rod_config.u_max_acceptable) {
		++transmitted;
	    }
	}

	const double efficiency = transmitted / ion_motions.size();

	resutls.emplace_back(m, efficiency);
    }

    return resutls;
}


} // namespace qmfcalc
