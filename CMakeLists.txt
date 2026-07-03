# -----
# Fundamental settings
# -----

cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(qmfcalc VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(QMFCALC_BUILD_EXECUTABLES "Build ROOT-backed command line executables" ON)
option(QMFCALC_BUILD_DOCS "Build Doxygen documentation target" ON)
option(QMFCALC_BUILD_PYTHON "Build Python extension module" OFF)

if(QMFCALC_BUILD_PYTHON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


# -----
# External packages
# -----

include(FetchContent)

if(QMFCALC_BUILD_EXECUTABLES)
    # argparse: command line argument parsing (header-only).
    FetchContent_Declare(
	argparse
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG v3.2
    )

    # fkYAML: YAML serialization for result output (header-only).
    FetchContent_Declare(
	fkYAML
	GIT_REPOSITORY https://github.com/fktn-k/fkYAML.git
	GIT_TAG v0.4.2
    )

    FetchContent_MakeAvailable(argparse fkYAML)

    find_package(ROOT REQUIRED COMPONENTS Core Hist Gpad Tree RIO)

    # roothelper (CERN ROOT data management and plot formatting) provides the
    # RootHelper INTERFACE target. Its own CMake calls find_package(ROOT).
    add_subdirectory(${PROJECT_SOURCE_DIR}/ext/roothelper)
endif()

if(QMFCALC_BUILD_PYTHON)
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG QUIET)

    if(NOT pybind11_FOUND)
	FetchContent_Declare(
	    pybind11
	    GIT_REPOSITORY https://github.com/pybind/pybind11.git
	    GIT_TAG v2.13.6
	)
	FetchContent_MakeAvailable(pybind11)
    endif()
endif()


# -----
# Sub-directories
# -----

add_subdirectory(${PROJECT_SOURCE_DIR}/core)

if(QMFCALC_BUILD_DOCS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/doc)
endif()

if(QMFCALC_BUILD_EXECUTABLES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/exec)
endif()

if(QMFCALC_BUILD_PYTHON)
    add_subdirectory(${PROJECT_SOURCE_DIR}/python)
endif()
