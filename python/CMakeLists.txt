pybind11_add_module(qmscalc_python MODULE qmscalc_pybind.cpp)

target_link_libraries(qmscalc_python PRIVATE
    qmscalc::VoltageCalculator
)

target_compile_features(qmscalc_python PRIVATE cxx_std_17)

target_compile_definitions(qmscalc_python PRIVATE
    QMSCALC_VERSION="${PROJECT_VERSION}"
)

set_target_properties(qmscalc_python PROPERTIES
    OUTPUT_NAME qmscalc
)

install(TARGETS qmscalc_python LIBRARY DESTINATION .)
