#include <cmath>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>

#include <qmscalc/voltage_calculator.h>


namespace py = pybind11;
using namespace pybind11::literals;


namespace {


constexpr double kAtomicMassUnit = 1.66053906660e-27; // [kg]
constexpr double kMilliMetre = 1e-3;                  // [m]
constexpr double kMegaHertz = 1e6;                    // [Hz]


void require_positive(const char* name, const double value)
{
    if (!std::isfinite(value) || value <= 0) {
	throw std::invalid_argument(std::string(name) + " must be finite and positive");
    }
}


py::dict calculate_voltages(const double r0_mm,
			    const double target_m_over_q_amu_per_z,
			    const double resolution,
			    const double rf_frequency_mhz)
{
    require_positive("r0_mm", r0_mm);
    require_positive("target_m_over_q_amu_per_z", target_m_over_q_amu_per_z);
    require_positive("resolution", resolution);
    require_positive("rf_frequency_mhz", rf_frequency_mhz);

    // The C++ kernel is written for singly charged ions. Supplying m/Z in amu/Z
    // is equivalent to using an effective singly charged mass of (m/Z) amu.
    const double effective_mass_kg = target_m_over_q_amu_per_z * kAtomicMassUnit;
    const double delta_mass_kg = effective_mass_kg / resolution;
    const double r0_m = r0_mm * kMilliMetre;
    const double rf_frequency_hz = rf_frequency_mhz * kMegaHertz;

    const qmscalc::VoltageSolution solution =
	qmscalc::compute_target_voltages(effective_mass_kg, delta_mass_kg, r0_m, rf_frequency_hz);

    if (!solution.transmitted) {
	throw std::runtime_error("requested resolution is unreachable; no operating point exists");
    }

    return py::dict(
	"rf_voltage_v"_a = solution.V,
	"dc_voltage_v"_a = solution.U
    );
}


} // namespace


PYBIND11_MODULE(qmscalc, m)
{
    m.doc() = "Python bindings for QMS voltage calculations.";
    m.attr("__version__") = QMSCALC_VERSION;

    m.def(
	"calculate_voltages",
	&calculate_voltages,
	"r0_mm"_a,
	"target_m_over_q_amu_per_z"_a,
	"resolution"_a,
	"rf_frequency_mhz"_a,
	R"pbdoc(
Calculate QMS RF and DC electrode voltages.

Args:
    r0_mm: Field radius from central axis to rod surface [mm].
    target_m_over_q_amu_per_z: Target mass-to-charge value [amu/Z].
    resolution: Resolving power R = (m/q) / delta(m/q).
    rf_frequency_mhz: RF drive frequency [MHz].

Returns:
    dict: ``{"rf_voltage_v": ..., "dc_voltage_v": ...}``.

The RF voltage is the per-rod peak-to-peak voltage. The DC voltage is
pole-to-pole, with rods at +/-dc_voltage_v/2.
)pbdoc");
}
