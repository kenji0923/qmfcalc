# ----- Custom function definition start -----

# Add the standard include directory.
function(add_include_directory_std target_name scope)
    target_include_directories(${target_name} ${scope}
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/${PROJECT_NAME}>
    )
endfunction()

# Add library with a standard filenames
# The third argument can be used for specifying library type (STATIC|SHARED|INTERFACE)
function(add_library_std target_name library_file_name)
    set(library_type "")

    if(${ARGC} GREATER 2)
	set(library_type ${ARGV2})
    endif()

    set(include_scope "PUBLIC")

    if("${library_type}" STREQUAL "INTERFACE")
	add_library(${target_name} ${library_type} include/${PROJECT_NAME}/${library_file_name}.h)
	set(include_scope "INTERFACE")
    else()
	add_library(${target_name} ${library_type} ${library_file_name}.cpp include/${PROJECT_NAME}/${library_file_name}.h)
    endif()

    add_include_directory_std(${target_name} ${include_scope})

    add_library(${PROJECT_NAME}::${target_name} ALIAS ${target_name})
endfunction()

# ----- Function definiion end -----

add_library_std(StabilityDiagram stability_diagram INTERFACE)

add_library_std(ResolutionEstimator resolution_estimator)
target_link_libraries(ResolutionEstimator PUBLIC
    StabilityDiagram
)

add_library_std(VoltageCalculator voltage_calculator)
target_link_libraries(VoltageCalculator PUBLIC
    ResolutionEstimator
)
