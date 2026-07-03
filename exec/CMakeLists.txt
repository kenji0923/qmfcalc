add_executable(analyze_resolution analyze_resolution.cc)
target_link_libraries(analyze_resolution PRIVATE
    qmfcalc::ResolutionEstimator
    qmfcalc::StabilityDiagram
    RootHelper
    argparse::argparse
    fkYAML::fkYAML
    ROOT::Tree
    ROOT::RIO
)


add_executable(calculate_voltage calculate_voltage.cc)
target_link_libraries(calculate_voltage PRIVATE
    qmfcalc::VoltageCalculator
    qmfcalc::ResolutionEstimator
    qmfcalc::StabilityDiagram
    RootHelper
    argparse::argparse
    fkYAML::fkYAML
    ROOT::Tree
    ROOT::RIO
)
