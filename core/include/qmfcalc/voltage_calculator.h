#ifndef QMFCALC_VOLTAGE_CALCULATOR_H
#define QMFCALC_VOLTAGE_CALCULATOR_H


namespace qmfcalc {


// Elementary charge [C].
constexpr double kElementaryCharge = 1.602176634e-19;


// Notation (all SI: m [kg], omega [rad/s], r0 [m], voltages [V]).
//
// r0 is the distance from the central axis to the rod surface (the field /
// inscribed radius), entering as r0^2 below. The per-rod electrode potential is
//   rod pair X:  +U/2 + (V/2) cos(omega t)
//   rod pair Y:  -U/2 - (V/2) cos(omega t)
// which yields the Mathieu parameters
//   a = 4 e U / (m omega^2 r0^2)   -> U = a * E_osc / (4 e)
//   q = 2 e V / (m omega^2 r0^2)   -> V = q * E_osc / (2 e)
// so U is the pole-to-pole DC voltage (rods at +/-U/2) and V is the per-rod
// peak-to-peak RF voltage (each rod swings +/-V/2). E_osc = m (omega r0)^2.


// Characteristic oscillation energy E_osc = m (omega r0)^2 [J].
double get_oscillation_energy(double m, double omega, double r0);


// Pole-to-pole DC voltage U [V] for Mathieu parameter a (rods at +/-U/2).
double get_dc_voltage(double m, double omega, double r0, double a);


// Per-rod peak-to-peak RF voltage V [V] for Mathieu parameter q (rods swing +/-V/2).
double get_rf_voltage(double m, double omega, double r0, double q);


// Target electrode voltages that transmit a chosen mass center at a chosen
// resolution.
struct VoltageSolution
{
    bool transmitted;  // false if the requested resolution is unreachable
    double resolution; // requested R = m / delta_m
    double slope;      // a/q of the operating line
    double q;          // operating point q (transmitted-mass-center, = q_center)
    double a;          // operating point a (= a_center)
    double U;          // pole-to-pole DC voltage [V]
    double V;          // per-rod peak-to-peak RF voltage [V]
};


// Operating point recovered from electrode voltages.
struct MassResolutionSolution
{
    bool transmitted;  // false if the voltage ratio gives no stability window
    double mass;       // transmitted effective singly charged mass [kg]
    double resolution; // recovered R = m / delta_m
    double slope;      // a/q of the operating line
    double q;          // operating point q (transmitted-mass-center, = q_center)
    double a;          // operating point a (= a_center)
    double U;          // pole-to-pole DC voltage [V]
    double V;          // per-rod peak-to-peak RF voltage [V]
};


// Given the mass center m [kg], resolution width delta_m [kg], field radius
// r0 [m] and RF frequency f [Hz], solve for the DC (U) and RF (V) voltages.
// The scan-line slope is recovered from the resolution estimator, the operating
// point is taken at the transmitted-mass-center (window midpoint), and the
// voltages follow from the Mathieu definitions above.
VoltageSolution compute_target_voltages(double m, double delta_m, double r0, double f);


// Given the pole-to-pole DC voltage U [V], per-rod peak-to-peak RF voltage
// V [V], field radius r0 [m] and RF frequency f [Hz], solve for the
// transmitted mass center and resolution.
MassResolutionSolution compute_mass_resolution_from_voltages(double U, double V, double r0,
							     double f);


} // namespace qmfcalc


#endif
