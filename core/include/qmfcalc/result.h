#ifndef QMFCALC_RESULT_H
#define QMFCALC_RESULT_H


namespace qmfcalc {


namespace Result {


struct Efficiency
{
    Efficiency(const double mass, const double transmission) : mass(mass), transmission(transmission) { }

    const double mass;
    const double transmission;
};


} // namespace qmfcalc::Result


} // namespace qmfcalc


#endif
