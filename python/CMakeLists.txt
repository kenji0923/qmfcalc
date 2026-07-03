pybind11_add_module(qmfcalc_python MODULE qmfcalc_pybind.cpp)

target_link_libraries(qmfcalc_python PRIVATE
    qmfcalc::VoltageCalculator
)

target_compile_features(qmfcalc_python PRIVATE cxx_std_17)

target_compile_definitions(qmfcalc_python PRIVATE
    QMFCALC_VERSION="${PROJECT_VERSION}"
)

set_target_properties(qmfcalc_python PROPERTIES
    OUTPUT_NAME qmfcalc
)

install(TARGETS qmfcalc_python
    LIBRARY DESTINATION .
    RUNTIME DESTINATION .
)
