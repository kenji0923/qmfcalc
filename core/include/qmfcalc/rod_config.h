#ifndef QMFCALC_ROD_CONFIG_H
#define QMFCALC_ROD_CONFIG_H


namespace qmfcalc {


struct RodConfig
{
    RodConfig(const double u_max_acceptable, const double length) : u_max_acceptable(u_max_acceptable), length(length) { }

    const double u_max_acceptable;
    const double length;
};


} // namespace qmfcalc


#endif
