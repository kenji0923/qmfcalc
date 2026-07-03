#include <qmfcalc/resolution_estimator.h>

#include <cmath>

#include <qmfcalc/stability_diagram.h>


namespace qmfcalc {


namespace {


constexpr double kBisectionTolerance = 1e-12;
constexpr int kBisectionMaxIteration = 200;


// Apex slope a_peak / q_peak. A scan line with this slope just grazes the tip;
// steeper lines clear the stability region entirely.
double get_apex_slope()
{
    return get_first_stability_apex_a() / kFirstStabilityQPeak;
}


// Bisection root finder for a monotonic-sign function on [lo, hi], which must
// bracket exactly one sign change.
template <class Function>
double bisect(Function f, double lo, double hi)
{
    double f_lo = f(lo);

    for (int i = 0; i < kBisectionMaxIteration; ++i) {
	const double mid = 0.5 * (lo + hi);
	const double f_mid = f(mid);

	if (std::abs(f_mid) < kBisectionTolerance || 0.5 * (hi - lo) < kBisectionTolerance) {
	    return mid;
	}

	if ((f_lo < 0) == (f_mid < 0)) {
	    lo = mid;
	    f_lo = f_mid;
	} else {
	    hi = mid;
	}
    }

    return 0.5 * (lo + hi);
}


ResolutionEstimate make_untransmitted(const double slope)
{
    return ResolutionEstimate{false, slope, 0, 0, 0, 0, 0, 0};
}


} // namespace


ResolutionEstimate estimate_mass_resolution_from_slope(const double slope)
{
    if (slope <= 0 || slope >= get_apex_slope()) {
	return make_untransmitted(slope);
    }

    // g(q) = boundary(q) - slope * q. Roots are the scan-line crossings.
    const auto g = [slope](const double q) { return get_first_stability_boundary(q) - slope * q; };

    // Rising a0 branch: g < 0 just above the origin, g > 0 at the apex.
    const double q_low = bisect(g, 1e-9, kFirstStabilityQPeak);

    // Falling b1 branch: g > 0 at the apex, g < 0 at the right end of the tip.
    const double q_high = bisect(g, kFirstStabilityQPeak, kFirstStabilityQMax);

    const double q_center = 0.5 * (q_low + q_high);
    const double delta_q = q_high - q_low;

    ResolutionEstimate result;
    result.transmitted = true;
    result.slope = slope;
    result.q_low = q_low;
    result.q_high = q_high;
    result.q_center = q_center;
    result.a_center = slope * q_center;
    result.delta_q = delta_q;
    result.mass_resolution = q_center / delta_q;

    return result;
}


ResolutionEstimate estimate_mass_resolution(const double q, const double a)
{
    if (q == 0) {
	return make_untransmitted(0);
    }

    return estimate_mass_resolution_from_slope(a / q);
}


double find_slope_for_mass_resolution(const double target_resolution)
{
    const double apex_slope = get_apex_slope();

    // Resolution increases monotonically with slope; evaluate the floor near
    // slope -> 0 to reject unreachable targets.
    const double min_resolution = estimate_mass_resolution_from_slope(1e-6).mass_resolution;

    if (target_resolution <= min_resolution) {
	return -1;
    }

    // r(slope) - target is monotonically increasing in slope, so bisect on the
    // open interval (0, apex_slope).
    const auto r = [target_resolution](const double slope) {
	return estimate_mass_resolution_from_slope(slope).mass_resolution - target_resolution;
    };

    return bisect(r, 1e-6, apex_slope - 1e-9);
}


} // namespace qmfcalc
