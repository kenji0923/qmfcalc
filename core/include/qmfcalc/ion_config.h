#ifndef QMFCALC_ION_CONFIG_H
#define QMFCALC_ION_CONFIG_H


#include <vector>


namespace qmfcalc {


struct IonMotion
{
    IonMotion(
	    const double x,
	    const double y,
	    const double v_x,
	    const double v_y,
	    const double v_z
	)
    :	 x(x),
	y(y),
	v_x(v_x),
	v_y(v_y),
	v_z(v_z)
    { }

    double x;
    double y;
    double v_x;
    double v_y;
    double v_z;
};


struct IonSet
{
    IonSet(
	    const double mass,
	    const std::vector<IonMotion>& motions
	)
    :	mass(mass),
    	motions(motions)
    { }

    const double mass;
    const std::vector<IonMotion> motions;
};


struct PhaseSpacePoint
{
    static double get_u_dot(const double omega, const double v) { return 2. / omega * v; }

    PhaseSpacePoint(const double omega, const double u, const double v) : omega(omega), u(u), u_dot(get_u_dot(omega, v)) { }

    const double omega;
    double u;
    double u_dot;
};


} // namespace qmfcalc


#endif
