#ifndef QMSCALC_RESOLUTION_ESTIMATOR_H
#define QMSCALC_RESOLUTION_ESTIMATOR_H


namespace qmscalc {


// Result of estimating the mass resolution for a single operating line.
//
// In a quadrupole mass filter every mass sits on a scan line through the origin
// of the (q, a) diagram with slope k = a/q = 2U/V (mass independent for fixed
// rod voltages, frequency and geometry). The line crosses the tip of the first
// stability region at q_low (rising a0 branch) and q_high (falling b1 branch).
// Because q is proportional to 1/m, the transmitted mass band corresponds to the
// q-window delta_q = q_high - q_low, giving m/dm = q_center / delta_q.
struct ResolutionEstimate
{
    bool transmitted;       // false if the scan line clears the apex (no window)
    double slope;           // k = a/q of the operating line
    double q_low;           // crossing on the rising a0 branch
    double q_high;          // crossing on the falling b1 branch
    double q_center;        // (q_low + q_high) / 2 : transmitted mass center in q
    double a_center;        // slope * q_center
    double delta_q;         // q_high - q_low
    double mass_resolution; // m/dm = q_center / delta_q
};


// Estimate the mass resolution from the line through the origin and (q, a).
// Only the slope a/q matters; q is required to be non-zero.
ResolutionEstimate estimate_mass_resolution(double q, double a);


// Estimate the mass resolution directly from the operating-line slope k = a/q.
ResolutionEstimate estimate_mass_resolution_from_slope(double slope);


// Inverse of the estimator: the operating-line slope (i.e. the 2U/V ratio)
// whose window yields target_resolution. The resolution increases monotonically
// as the slope approaches the apex slope, so a bisection on (0, apex slope) is
// used. Returns a negative value if target_resolution is below the minimum
// achievable. This is the kernel of the future RF/DC amplitude solver.
double find_slope_for_mass_resolution(double target_resolution);


} // namespace qmscalc


#endif
