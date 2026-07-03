#ifndef QMFCALC_STABILITY_DIAGRAM_H
#define QMFCALC_STABILITY_DIAGRAM_H


#include <cmath>


namespace qmfcalc {


// q value of the apex (tip) of the first stability region, where the rising
// a0 branch and the falling b1 branch meet.
constexpr double kFirstStabilityQPeak = 0.706252;

// q value where the falling b1 branch returns to a = 0 (right end of the tip).
constexpr double kFirstStabilityQMax = 0.909104;


// Upper boundary of the first stability region of the Mathieu equation as a
// single-valued function of q. The region's tip is bounded by two branches that
// meet at (kFirstStabilityQPeak, apex):
//   - q <  q_peak : rising a0 characteristic curve
//   - q >= q_peak : falling b1 characteristic curve
// Asymptotic series expansions are used for each branch.
inline double get_first_stability_boundary(const double q, const double q_peak = kFirstStabilityQPeak)
{
    const double q2 = std::pow(q, 2);
    const double q4 = std::pow(q, 4);
    const double q6 = std::pow(q, 6);
    const double q8 = std::pow(q, 8);

    if (q < q_peak) {
	const double a0 = -q2 / 2 + 7 * q4 / 128 - 29 * q6 / 2304 + 68687 * q8 / 18874368;
	return -a0;
    } else {
	const double q3 = std::pow(q, 3);
	const double q5 = std::pow(q, 5);
	const double q7 = std::pow(q, 7);

	const double b1 = 1 - q - q2 / 8 + q3 / 64 + q4 / 1536 + 11 * q5 / 36864 + 49 * q6 / 589824 - 55 * q7 / 9437184 - 265 * q8 / 113246208;

	return b1;
    }
}


// a value of the apex of the first stability region (a_peak ~ 0.237).
inline double get_first_stability_apex_a(const double q_peak = kFirstStabilityQPeak)
{
    return get_first_stability_boundary(q_peak, q_peak);
}


} // namespace qmfcalc


#endif
