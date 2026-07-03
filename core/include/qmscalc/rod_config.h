#ifndef QMSCALC_ROD_CONFIG_H
#define QMSCALC_ROD_CONFIG_H


namespace qmscalc {


struct RodConfig
{
    RodConfig(const double u_max_acceptable, const double length) : u_max_acceptable(u_max_acceptable), length(length) { }

    const double u_max_acceptable;
    const double length;
};


} // namespace qmscalc


#endif
