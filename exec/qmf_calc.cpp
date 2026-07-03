#include "ROOT_helper/ROOT_helper.h"

#include <vector>

#include <TCanvas.h>
#include <TF1.h>
#include <TMath.h>


namespace u
{
    constexpr double m = 1;
    constexpr double kg = 1;
    constexpr double s = 1;
    constexpr double A = 1;
    constexpr double K = 1;


    constexpr double Hz = 1 / s;
    constexpr double J = kg * m * m / s / s;
    constexpr double C = A * s;
    constexpr double V = J / C;
    constexpr double T = kg / A / s / s;


    constexpr double eV = 1.602176634e-19 * J;


    constexpr double MHz = 1e6 * Hz;
    constexpr double cm = 1e-2 * m;
    constexpr double mm = 1e-3 * m;
    constexpr double us = 1e-6 * s;
    constexpr double ns = 1e-9 * s;
    constexpr double kV = 1e3 * V;


    constexpr double MeV = 1e6 * eV;
    constexpr double keV = 1e3 * eV;


    constexpr double c = 299792458. * m / s;
    constexpr double e = 1.602176634e-19; // TODO
    constexpr double u = 931.49410372 * MeV / c / c;
    constexpr double kB = 1.380649e-23 * J / K;
};


namespace rh = ROOT_helper;


double get_first_stability_boundary(const double* x, const double* p);

double get_oscillation_energy(const double m, const double omega, const double r0);

double get_U(const double m, const double omega, const double r0, const double a);

double get_V(const double m, const double omega, const double r0, const double q);

double get_mass_resolution(const double E_k, const double L, const double f, const double C_res);


int qmf_calc()
{
    rh::prepare();

    rh::DataSaver data_saver("data_qmf_calc");


    TF1* f_first_stability_boundary = new TF1("f_first_stability_boundary", get_first_stability_boundary, 0, 0.909104, 1);
    f_first_stability_boundary->SetTitle("Fist stability boundary");
    f_first_stability_boundary->SetNpx(1024);
    f_first_stability_boundary->SetLineColor(kBlack);
    f_first_stability_boundary->SetParNames("q_peak");

    const double q_peak = 0.706252;
    f_first_stability_boundary->SetParameter(0, q_peak);

    const double a_peak = f_first_stability_boundary->Eval(q_peak);
    printf("a_peak=%.6lf\n", a_peak);


    TCanvas* c_stability = rh::create_canvas("c_stability", "Stability");
    f_first_stability_boundary->Draw("L");

    rh::set_axes(f_first_stability_boundary);
    f_first_stability_boundary->GetXaxis()->SetTitle("q");
    f_first_stability_boundary->GetYaxis()->SetTitle("a");
    rh::increase_right_margin();


    data_saver.write_canvas(c_stability);


    struct Particle
    {
	const std::string name;
	const double mass;
	const int valence;
    };


    const std::vector<Particle> particle {
	{ "Th-232", 232.0377 * u::u, 1},
	{ "$\\mathrm{^{232}Th^{16}O}$", 248.0326 * u::u, 1},
    };


    {
	const double m = 250 * u::u;
	const double omega = TMath::TwoPi() * 1 * u::MHz;
	const double r0 = 15 * u::mm;

	const double U = get_U(m, omega, r0, a_peak);
	const double V = get_V(m, omega, r0, q_peak);

	printf("U=%gV, V=%gV at the peak\n", U / u::V, V / u::V);

	TF1* f_U = new TF1("f_U",
	    [&](const double* x, const double*p)
	    {
		const double r0 = x[0] / 2 * u::mm;
		const double m = p[0];
		const double omega = p[1];

		return get_U(m, omega, r0, a_peak) / u::kV;
	    },
	    5, 15,
	    2
	);
	f_U->SetTitle("U vs 2r0(mm)");
	f_U->SetNpx(1024);
	f_U->SetLineColor(kBlack);
	f_U->SetLineWidth(1);
	f_U->SetParNames("m", "omega");
	f_U->SetParameters(m, omega);

	TF1* f_V = new TF1("f_V",
	    [&](const double* x, const double*p)
	    {
		const double r0 = x[0] / 2 * u::mm;
		const double m = p[0];
		const double omega = p[1];

		return get_V(m, omega, r0, q_peak) / u::kV;
	    },
	    5, 15,
	    2
	);
	f_V->SetTitle("V vs 2r0(mm)");
	f_V->SetNpx(1024);
	f_V->SetLineColor(kBlack);
	f_V->SetLineWidth(1);
	f_V->SetParNames("m", "omega");
	f_V->SetParameters(m, omega);

	auto set_frequency = [&f_U, f_V](const double freq)
	{
	    f_U->SetParameter(1, TMath::TwoPi() * freq);
	    f_U->SetTitle(Form("f=%.1lfMHz", freq / u::MHz));

	    f_V->SetParameter(1, TMath::TwoPi() * freq);
	    f_V->SetTitle(Form("f=%.1lfMHz", freq / u::MHz));
	};

	const std::vector<double> frequencies { 0.8 * u::MHz, 0.6 * u::MHz };


	TCanvas* c_required_bias = rh::create_canvas("c_required_bias", "Required voltages at stability peak", 2, 1);
	
	c_required_bias->cd(1);

	{
	    f_U->SetLineColor(kBlack);
	    set_frequency(1 * u::MHz);

	    TH1* h_axis = f_U->DrawCopy("L")->GetHistogram();

	    rh::set_axes(h_axis);
	    h_axis->SetMinimum(0);
	    rh::increase_right_margin();
	    h_axis->GetXaxis()->SetTitle("Electrode surface distance (mm)");
	    h_axis->GetYaxis()->SetTitle("DC bias voltage |U/2| (kV)");

	    for (int i = 0; i < frequencies.size(); ++i) {
		set_frequency(frequencies[i]);
		f_U->SetLineColor(2 + i);
		f_U->DrawCopy("LSAME");
	    }

	    ROOT_helper::put_legend(ROOT_helper::LegendPosition::TopLeft, "L");
	}
	
	c_required_bias->cd(2);

	{
	    f_V->SetLineColor(kBlack);
	    set_frequency(1 * u::MHz);

	    TH1* h_axis = f_V->DrawCopy("L")->GetHistogram();

	    rh::set_axes(h_axis);
	    h_axis->SetMinimum(0);
	    rh::increase_right_margin();
	    h_axis->GetXaxis()->SetTitle("Electrode surface distance (mm)");
	    h_axis->GetYaxis()->SetTitle("RF peak-to-peak voltage V (kV)");

	    for (int i = 0; i < frequencies.size(); ++i) {
		set_frequency(frequencies[i]);
		f_V->SetLineColor(2 + i);
		f_V->DrawCopy("LSAME");
	    }

	    ROOT_helper::put_legend(ROOT_helper::LegendPosition::TopLeft, "L");
	}

	data_saver.write_canvas(c_required_bias);


	const double ellipse_edge_approx = 0.035;
	
	const double u_perp_max = r0 * ellipse_edge_approx;
	const double v_perp_max = omega / 2 * r0 * ellipse_edge_approx;
	const double Ek_perp_max = m * std::pow(v_perp_max, 2) / 2;

	printf("u_perp_max=%gmm\n", u_perp_max / (u::mm));
	printf("v_perp_max=%gm/s, Ek_perp_max=%geV\n", v_perp_max / (u::m / u::s), Ek_perp_max / (u::eV));
    }


    /**
     * Mass resolution by the finite length
     */

    {
	TF1* func_mass_resolution = new TF1("func_mass_resolution",
		[](const double* x, const double* p)
		{
		    const double L = x[0] * u::cm;

		    const double E_k = p[0];
		    const double f = p[1];
		    const double C_res = p[2];

		    const double base_mass = p[3];

		    return base_mass / get_mass_resolution(E_k, L, f, C_res);
		},
		10, 40,
		4
	    );

	func_mass_resolution->SetLineColor(kBlack);
	func_mass_resolution->SetLineWidth(1);
	func_mass_resolution->SetParNames("E_k", "f", "C_res", "m");
	func_mass_resolution->GetXaxis()->SetTitle("Length (cm)");
	func_mass_resolution->GetYaxis()->SetTitle("m/#Deltam");

	func_mass_resolution->SetParameters(10 * u::eV, 1 * u::MHz, 20, 250 * u::u);

	auto set_frequency = [&func_mass_resolution](const double freq)
	{
	    func_mass_resolution->SetParameter(1, freq);
	    func_mass_resolution->SetTitle(Form("f=%.1lfMHz", freq / u::MHz));
	};

	TCanvas* c_mass_resolution = ROOT_helper::create_canvas("c_mass_resolution", "c_mass_resolution");

	gPad->SetLogy();

	set_frequency(1 * u::MHz);
	
	TH1* h_axis = func_mass_resolution->DrawCopy("L")->GetHistogram();
	h_axis->GetXaxis()->SetTitle(func_mass_resolution->GetXaxis()->GetTitle());
	h_axis->GetYaxis()->SetTitle(func_mass_resolution->GetYaxis()->GetTitle());

	ROOT_helper::set_axes(h_axis);
	ROOT_helper::increase_right_margin(1);

	const std::vector<double> frequencies { 0.8 * u::MHz, 0.6 * u::MHz };

	for (int i = 0; i < frequencies.size(); ++i) {
	    set_frequency(frequencies[i]);
	    func_mass_resolution->SetLineColor(2 + i);
	    func_mass_resolution->DrawCopy("LSAME");
	}

	ROOT_helper::put_legend(ROOT_helper::LegendPosition::TopLeft, "L");

	ROOT_helper::draw_horizontal_line(100)->SetLineStyle(2);
	ROOT_helper::draw_horizontal_line(200)->SetLineStyle(2);
	ROOT_helper::draw_horizontal_line(300)->SetLineStyle(2);

	data_saver.write_canvas(c_mass_resolution);
    }


    return 0;
}


double get_first_stability_boundary(const double* x, const double* p)
{
    const double q = x[0];
    const double q_peak = p[0];

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


double get_oscillation_energy(const double m, const double omega, const double r0)
{
    return m * std::pow(omega * r0, 2);
}


double get_U(const double m, const double omega, const double r0, const double a)
{
    // U is the pole-to-pole DC voltage (rods at +/-U/2): a = 4eU/(m omega^2 r0^2).
    return a * get_oscillation_energy(m, omega, r0) / 4 / u::e;

}


double get_V(const double m, const double omega, const double r0, const double q)
{
    return q * get_oscillation_energy(m, omega, r0) / 2 / u::e;
}


double get_mass_resolution(const double E_k, const double L, const double f, const double C_res)
{
    return 2 * C_res * E_k / std::pow(L * f, 2);
}
