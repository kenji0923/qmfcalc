#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <TDirectory.h>
#include <TF1.h>
#include <TGraph.h>
#include <TTree.h>

#include <argparse/argparse.hpp>
#include <fkYAML/node.hpp>
#include <roothelper/roothelper.h>

#include <qmscalc/stability_diagram.h>
#include <qmscalc/voltage_calculator.h>


namespace rh = roothelper;


namespace {


// Input unit conversions to SI.
constexpr double kAtomicMassUnit = 1.66053906660e-27; // [kg]
constexpr double kMilliMetre = 1e-3;                  // [m]
constexpr double kMegaHertz = 1e6;                    // [Hz]


// Requested target, in the natural input units, kept for reporting.
struct Input
{
    double mass_u;         // mass center [u]
    double delta_mass_u;   // resolution width [u]
    double r0_mm;          // field radius (axis to rod surface) [mm]
    double frequency_MHz;  // RF frequency [MHz]
};


// Wrap the stability boundary as a parameter-free TF1 for drawing.
double stability_boundary_tf1(const double* x, const double*)
{
    return qmscalc::get_first_stability_boundary(x[0]);
}


std::string default_output_directory(const Input& in)
{
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "voltage_m%.3f_dm%.3f_r0%.3f_f%.3f",
		  in.mass_u, in.delta_mass_u, in.r0_mm, in.frequency_MHz);
    return buffer;
}


void write_result_yaml(const std::filesystem::path& path, const Input& in,
		       const qmscalc::VoltageSolution& sol)
{
    fkyaml::node input = fkyaml::node::mapping();
    input["mass_u"] = in.mass_u;
    input["delta_mass_u"] = in.delta_mass_u;
    input["r0_mm"] = in.r0_mm;                  // axis-to-rod-surface field radius
    input["frequency_MHz"] = in.frequency_MHz;
    input["resolution"] = sol.resolution;       // R = m / delta_m

    fkyaml::node result = fkyaml::node::mapping();
    result["transmitted"] = sol.transmitted;
    result["slope_a_over_q"] = sol.slope;
    result["q"] = sol.q;
    result["a"] = sol.a;
    result["U_volt"] = sol.U;                    // pole-to-pole DC (rods at +/-U/2)
    result["V_volt"] = sol.V;                    // per-rod peak-to-peak RF (rods +/-V/2)

    fkyaml::node root = fkyaml::node::mapping();
    root["input"] = input;
    root["result"] = result;

    std::ofstream ofs(path);
    ofs << root << std::endl;
}


// Build, fill and write the single-entry result TTree into the given ROOT
// directory (the data.root opened by DataSaver).
void write_result_tree(TDirectory* directory, const Input& in, const qmscalc::VoltageSolution& sol)
{
    double mass_u = in.mass_u;
    double delta_mass_u = in.delta_mass_u;
    double r0_mm = in.r0_mm;
    double frequency_MHz = in.frequency_MHz;
    double resolution = sol.resolution;
    int transmitted = sol.transmitted ? 1 : 0;
    double slope = sol.slope;
    double q = sol.q;
    double a = sol.a;
    double u_volt = sol.U;
    double v_volt = sol.V;

    TTree* tree = new TTree("result", "QMS target voltage solution");
    tree->SetDirectory(directory);

    tree->Branch("mass_u", &mass_u);
    tree->Branch("delta_mass_u", &delta_mass_u);
    tree->Branch("r0_mm", &r0_mm);
    tree->Branch("frequency_MHz", &frequency_MHz);
    tree->Branch("resolution", &resolution);
    tree->Branch("transmitted", &transmitted);
    tree->Branch("slope_a_over_q", &slope);
    tree->Branch("q", &q);
    tree->Branch("a", &a);
    tree->Branch("U_volt", &u_volt);
    tree->Branch("V_volt", &v_volt);

    tree->Fill();
    tree->Write();
}


} // namespace


int calculate_voltage(int argc, char** argv)
{
    argparse::ArgumentParser program("calculate_voltage");
    program.add_description(
	"Compute the target DC (U) and RF (V) electrode voltages for a quadrupole "
	"mass filter that transmits mass center m at resolution m/dm. r0 is the "
	"field radius (central axis to rod surface). U is the pole-to-pole DC "
	"(rods at +/-U/2); V is the per-rod peak-to-peak RF (rods swing +/-V/2).");

    program.add_argument("-m", "--mass")
	.metavar("U")
	.scan<'g', double>()
	.required()
	.help("mass center m [u]");
    program.add_argument("-dm", "--delta-mass")
	.metavar("U")
	.scan<'g', double>()
	.required()
	.help("resolution width delta_m [u] (resolution R = m/delta_m)");
    program.add_argument("-r0", "--r0")
	.metavar("MM")
	.scan<'g', double>()
	.required()
	.help("field radius r0 = axis-to-rod-surface distance [mm]");
    program.add_argument("-f", "--frequency")
	.metavar("MHZ")
	.scan<'g', double>()
	.required()
	.help("RF frequency f [MHz]");

    program.add_argument("-o", "--output")
	.metavar("DIR")
	.help("output directory (default: derived from the configuration)");

    try {
	program.parse_args(argc, argv);
    } catch (const std::exception& err) {
	std::cerr << err.what() << "\n\n" << program;
	return 1;
    }

    Input in;
    in.mass_u = program.get<double>("--mass");
    in.delta_mass_u = program.get<double>("--delta-mass");
    in.r0_mm = program.get<double>("--r0");
    in.frequency_MHz = program.get<double>("--frequency");

    if (in.mass_u <= 0 || in.delta_mass_u <= 0 || in.r0_mm <= 0 || in.frequency_MHz <= 0) {
	std::cerr << "error: m, dm, r0 and f must all be positive\n";
	return 1;
    }

    const qmscalc::VoltageSolution sol = qmscalc::compute_target_voltages(
	in.mass_u * kAtomicMassUnit, in.delta_mass_u * kAtomicMassUnit, in.r0_mm * kMilliMetre,
	in.frequency_MHz * kMegaHertz);

    const std::filesystem::path output_directory =
	program.is_used("--output") ? program.get<std::string>("--output") : default_output_directory(in);
    std::filesystem::create_directories(output_directory);

    // ----- stdout report -----
    printf("Input: m=%.4f u  dm=%.4f u  r0=%.4f mm  f=%.4f MHz  (R=m/dm=%.2f)\n",
	   in.mass_u, in.delta_mass_u, in.r0_mm, in.frequency_MHz, sol.resolution);
    if (!sol.transmitted) {
	printf("  -> requested resolution is unreachable; no operating point exists\n");
	printf("Output directory: %s\n", output_directory.c_str());
	write_result_yaml(output_directory / "result.yaml", in, sol);
	return 1;
    }
    printf("  operating point: q=%.6f  a=%.6f  (slope a/q=%.6f)\n", sol.q, sol.a, sol.slope);
    printf("  DC voltage U (pole-to-pole, rods +/-U/2): %.3f V = %.4f kV\n", sol.U, sol.U / 1e3);
    printf("  RF voltage V (per-rod peak-to-peak)     : %.3f V = %.4f kV\n", sol.V, sol.V / 1e3);
    printf("Output directory: %s\n", output_directory.c_str());

    // ----- persisted outputs -----
    write_result_yaml(output_directory / "result.yaml", in, sol);
    rh::Prepare();
    {
	// Open data.root via DataSaver, write the TTree into it, then the canvas.
	rh::DataSaver data_saver(output_directory);
	write_result_tree(gDirectory, in, sol);

	TCanvas* c_stability_scan = rh::CreateCanvas("c_stability_scan", "Operating point");

	const double a_peak = qmscalc::get_first_stability_apex_a();
	TF1* f_boundary = new TF1("f_boundary", stability_boundary_tf1, 0, qmscalc::kFirstStabilityQMax, 0);
	f_boundary->SetTitle("First stability region with operating point");
	f_boundary->SetNpx(1024);
	f_boundary->SetLineColor(kBlack);
	f_boundary->SetMinimum(0);
	f_boundary->SetMaximum(1.25 * a_peak);
	f_boundary->Draw("L");

	rh::SetAxes(f_boundary);
	f_boundary->GetXaxis()->SetTitle("q");
	f_boundary->GetYaxis()->SetTitle("a");

	TF1* f_line = new TF1("f_line", "[0]*x", 0, 1.10 * sol.q);
	f_line->SetParameter(0, sol.slope);
	f_line->SetLineColor(kRed + 1);
	f_line->SetLineStyle(2);
	f_line->Draw("L SAME");

	TGraph* g_point = new TGraph(1);
	g_point->SetPoint(0, sol.q, sol.a);
	g_point->SetMarkerStyle(20);
	g_point->SetMarkerColor(kRed + 1);
	g_point->SetMarkerSize(1.4);
	g_point->Draw("P SAME");

	data_saver.WriteCanvas(c_stability_scan);
    }

    return 0;
}


int main(int argc, char** argv)
{
    return calculate_voltage(argc, argv);
}
