add_executable(analyze_resolution analyze_resolution.cc)
target_link_libraries(analyze_resolution PRIVATE
    qmscalc::ResolutionEstimator
    qmscalc::StabilityDiagram
    RootHelper
    argparse::argparse
    fkYAML::fkYAML
    ROOT::Tree
    ROOT::RIO
)


add_executable(calculate_voltage calculate_voltage.cc)
target_link_libraries(calculate_voltage PRIVATE
    qmscalc::VoltageCalculator
    qmscalc::ResolutionEstimator
    qmscalc::StabilityDiagram
    RootHelper
    argparse::argparse
    fkYAML::fkYAML
    ROOT::Tree
    ROOT::RIO
)
