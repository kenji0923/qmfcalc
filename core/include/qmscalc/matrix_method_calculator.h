#ifndef QMSCALC_MATRIX_METHOD_CALCULATOR_H
#define QMSCALC_MATRIX_METHOD_CALCULATOR_H


#include <array>
#include <vector>

#include <qmscalc/ion_config.h>
#include <qmscalc/result.h>
#include <qmscalc/rod_config.h>


namespace qmscalc {


class MatrixMethodCalculator
{
public:
    // double get_efficiency(const double mass, const Result::PhaseSpacePoint& initial_point) const;

    double get_u_max_infinite_length(
	    const double mass,
	    const IonMotion& ion_motion,
	    const double rod_length
	) const;

    std::vector<Result::Efficiency> get_efficiency_mass_spectrum_infinite_length(
	    const std::array<double, 2>& mass_range,
	    const double mass_step,
	    const std::vector<IonMotion>& ion_motions,
	    const RodConfig& rod_config
	) const;

private:
    const double RF_omega_;
    const double RF_period_;
};


} // namespace qmscalc


#endif
