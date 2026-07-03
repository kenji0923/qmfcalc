#ifndef QMSCALC_RESULT_H
#define QMSCALC_RESULT_H


namespace qmscalc {


namespace Result {


struct Efficiency
{
    Efficiency(const double mass, const double transmission) : mass(mass), transmission(transmission) { }

    const double mass;
    const double transmission;
};


} // namespace qmscalc::Result


} // namespace qmscalc


#endif
