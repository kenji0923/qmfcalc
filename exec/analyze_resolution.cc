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

#include <qmscalc/resolution_estimator.h>
#include <qmscalc/stability_diagram.h>


namespace rh = roothelper;


namespace {


// Wrap the stability boundary as a parameter-free TF1 for drawing.
double stability_boundary_tf1(const double* x, const double*)
{
    return qmscalc::get_first_stability_boundary(x[0]);
}


// One resolved analysis configuration. The operating line through the origin is
// fully described by its slope a/q; when an explicit (q, a) point is given the
// individual coordinates are kept too, otherwise they are NaN.
struct Config
{
    bool has_qa;     // true when an explicit (q, a) pair was supplied
    double q;        // input q (NaN if only the ratio was given)
    double a;        // input a (NaN if only the ratio was given)
    double q_over_a; // ratio q/a (mirrors the --q-over-a argument)
    double slope;    // a/q : slope of the scan line used by the estimator
};


// Parse a "q,a" string such as "0.70,0.23".
std::pair<double, double> parse_q_and_a(const std::string& text)
{
    const std::size_t comma = text.find(',');
    if (comma == std::string::npos) {
	throw std::runtime_error("--q_and_a expects 'q,a' (e.g. 0.70,0.23), got: " + text);
    }

    const double q = std::stod(text.substr(0, comma));
    const double a = std::stod(text.substr(comma + 1));
    return {q, a};
}


// Default output directory derived from the configuration.
std::string default_output_directory(const Config& cfg)
{
    char buffer[256];
    if (cfg.has_qa) {
	std::snprintf(buffer, sizeof(buffer), "resolution_q%.3f_a%.3f_qovera%.3f",
		      cfg.q, cfg.a, cfg.q_over_a);
    } else {
	std::snprintf(buffer, sizeof(buffer), "resolution_qovera%.3f", cfg.q_over_a);
    }
    return buffer;
}


void write_result_yaml(const std::filesystem::path& path, const Config& cfg,
		       const qmscalc::ResolutionEstimate& est)
{
    // fkYAML does not auto-vivify mappings, so each level is created explicitly.
    fkyaml::node input = fkyaml::node::mapping();
    input["mode"] = cfg.has_qa ? "q_and_a" : "q_over_a";
    if (cfg.has_qa) {
	input["q"] = cfg.q;
	input["a"] = cfg.a;
    }
    input["q_over_a"] = cfg.q_over_a;
    input["slope_a_over_q"] = cfg.slope;

    fkyaml::node result = fkyaml::node::mapping();
    result["transmitted"] = est.transmitted;
    result["q_low"] = est.q_low;
    result["q_high"] = est.q_high;
    result["q_center"] = est.q_center;
    result["a_center"] = est.a_center;
    result["delta_q"] = est.delta_q;
    result["mass_resolution"] = est.mass_resolution;

    fkyaml::node root = fkyaml::node::mapping();
    root["input"] = input;
    root["result"] = result;

    std::ofstream ofs(path);
    ofs << root << std::endl;
}


// Build, fill and write the single-entry result TTree into the given ROOT
// directory (the data.root opened by DataSaver).
void write_result_tree(TDirectory* directory, const Config& cfg,
		       const qmscalc::ResolutionEstimate& est)
{
    double q = cfg.q;
    double a = cfg.a;
    double q_over_a = cfg.q_over_a;
    double slope = cfg.slope;
    int transmitted = est.transmitted ? 1 : 0;
    double q_low = est.q_low;
    double q_high = est.q_high;
    double q_center = est.q_center;
    double a_center = est.a_center;
    double delta_q = est.delta_q;
    double mass_resolution = est.mass_resolution;

    TTree* tree = new TTree("result", "QMS mass resolution estimate");
    tree->SetDirectory(directory);

    tree->Branch("q", &q);
    tree->Branch("a", &a);
    tree->Branch("q_over_a", &q_over_a);
    tree->Branch("slope_a_over_q", &slope);
    tree->Branch("transmitted", &transmitted);
    tree->Branch("q_low", &q_low);
    tree->Branch("q_high", &q_high);
    tree->Branch("q_center", &q_center);
    tree->Branch("a_center", &a_center);
    tree->Branch("delta_q", &delta_q);
    tree->Branch("mass_resolution", &mass_resolution);

    tree->Fill();
    tree->Write();
}


void write_plot(const std::filesystem::path& output_directory, const Config& cfg,
		const qmscalc::ResolutionEstimate& est)
{
    const double a_peak = qmscalc::get_first_stability_apex_a();

    rh::Prepare();
    rh::DataSaver data_saver(output_directory);

    // DataSaver has just opened data.root; it is the current ROOT directory.
    // Store the result TTree there alongside the canvas data.
    write_result_tree(gDirectory, cfg, est);

    TCanvas* c_stability_scan = rh::CreateCanvas("c_stability_scan", "Stability scan line");

    TF1* f_boundary = new TF1("f_boundary", stability_boundary_tf1, 0, qmscalc::kFirstStabilityQMax, 0);
    f_boundary->SetTitle("First stability region with scan line");
    f_boundary->SetNpx(1024);
    f_boundary->SetLineColor(kBlack);
    f_boundary->SetMinimum(0);
    f_boundary->SetMaximum(1.25 * a_peak);
    f_boundary->Draw("L");

    rh::SetAxes(f_boundary);
    f_boundary->GetXaxis()->SetTitle("q");
    f_boundary->GetYaxis()->SetTitle("a");

    if (est.transmitted) {
	// Scan line a = slope * q from the origin to past the high crossing.
	TF1* f_line = new TF1("f_line", "[0]*x", 0, 1.05 * est.q_high);
	f_line->SetParameter(0, est.slope);
	f_line->SetLineColor(kRed + 1);
	f_line->SetLineStyle(2);
	f_line->Draw("L SAME");

	// Mark the two crossing points.
	TGraph* g_cross = new TGraph(2);
	g_cross->SetPoint(0, est.q_low, est.slope * est.q_low);
	g_cross->SetPoint(1, est.q_high, est.slope * est.q_high);
	g_cross->SetMarkerStyle(20);
	g_cross->SetMarkerColor(kRed + 1);
	g_cross->SetMarkerSize(1.2);
	g_cross->Draw("P SAME");
    }

    data_saver.WriteCanvas(c_stability_scan);
}


} // namespace


int analyze_resolution(int argc, char** argv)
{
    argparse::ArgumentParser program("analyze_resolution");
    program.add_description(
	"Estimate the QMS mass resolution from a (q, a) operating point, or from "
	"the q/a ratio of the scan line, and save the stability diagram with the "
	"scan line drawn.");

    auto& mode_group = program.add_mutually_exclusive_group(/*required=*/true);
    mode_group.add_argument("-qa", "--q_and_a")
	.metavar("q,a")
	.help("operating point as 'q,a' (e.g. 0.70,0.23)");
    mode_group.add_argument("-qova", "--q-over-a")
	.metavar("RATIO")
	.scan<'g', double>()
	.help("scan-line ratio q/a (the estimator slope is a/q = 1/RATIO)");

    program.add_argument("-o", "--output")
	.metavar("DIR")
	.help("output directory (default: derived from the configuration)");

    try {
	program.parse_args(argc, argv);
    } catch (const std::exception& err) {
	std::cerr << err.what() << "\n\n" << program;
	return 1;
    }

    // Resolve the configuration from the mutually exclusive inputs.
    Config cfg;
    try {
	if (program.is_used("--q_and_a")) {
	    const auto [q, a] = parse_q_and_a(program.get<std::string>("--q_and_a"));
	    if (q == 0 || a == 0) {
		throw std::runtime_error("q and a must both be non-zero");
	    }
	    cfg.has_qa = true;
	    cfg.q = q;
	    cfg.a = a;
	    cfg.q_over_a = q / a;
	    cfg.slope = a / q;
	} else {
	    const double q_over_a = program.get<double>("--q-over-a");
	    if (q_over_a == 0) {
		throw std::runtime_error("--q-over-a must be non-zero");
	    }
	    cfg.has_qa = false;
	    cfg.q = std::nan("");
	    cfg.a = std::nan("");
	    cfg.q_over_a = q_over_a;
	    cfg.slope = 1.0 / q_over_a;
	}
    } catch (const std::exception& err) {
	std::cerr << "error: " << err.what() << "\n";
	return 1;
    }

    const qmscalc::ResolutionEstimate est = qmscalc::estimate_mass_resolution_from_slope(cfg.slope);

    const std::filesystem::path output_directory =
	program.is_used("--output") ? program.get<std::string>("--output") : default_output_directory(cfg);
    std::filesystem::create_directories(output_directory);

    // ----- stdout report -----
    const double a_peak = qmscalc::get_first_stability_apex_a();
    printf("First stability apex: q_peak=%.6f a_peak=%.6f (apex slope a/q=%.6f)\n",
	   qmscalc::kFirstStabilityQPeak, a_peak, a_peak / qmscalc::kFirstStabilityQPeak);
    if (cfg.has_qa) {
	printf("Input: q=%.6f a=%.6f  (q/a=%.6f, a/q=%.6f)\n", cfg.q, cfg.a, cfg.q_over_a, cfg.slope);
    } else {
	printf("Input: q/a=%.6f  (a/q=%.6f)\n", cfg.q_over_a, cfg.slope);
    }
    if (est.transmitted) {
	printf("  crossings  : q_low=%.6f  q_high=%.6f  delta_q=%.6f\n",
	       est.q_low, est.q_high, est.delta_q);
	printf("  mass center: q_center=%.6f  a_center=%.6f\n", est.q_center, est.a_center);
	printf("  resolution : m/dm = %.4f\n", est.mass_resolution);
    } else {
	printf("  -> scan line clears the apex: no transmission window\n");
    }
    printf("Output directory: %s\n", output_directory.c_str());

    // ----- persisted outputs -----
    // The result TTree is written into data.root by write_plot (via DataSaver).
    write_result_yaml(output_directory / "result.yaml", cfg, est);
    write_plot(output_directory, cfg, est);

    return 0;
}


int main(int argc, char** argv)
{
    return analyze_resolution(argc, argv);
}
