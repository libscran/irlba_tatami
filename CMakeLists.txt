cmake_minimum_required(VERSION 3.14...3.25)

project(
    irlba_tatami
    VERSION 0.1.1
    DESCRIPTION "Wrap tatami matrices for irlba"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Library
add_library(irlba_tatami INTERFACE)
add_library(libscran::irlba_tatami ALIAS irlba_tatami)

target_include_directories(irlba_tatami INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libscran>)
target_compile_features(irlba_tatami INTERFACE cxx_std_17)

# Dependencies
option(IRLBA_FETCH_EXTERN "Automatically fetch CppIrlba's dependencies." ON)
if(IRLBA_FETCH_EXTERN)
    add_subdirectory(extern)
else()
    find_package(tatami_tatami 4.0.0 CONFIG REQUIRED)
    find_package(tatami_tatami_mult 0.1.0 CONFIG REQUIRED)
    find_package(ltla_irlba 3.0.0 CONFIG REQUIRED)
    find_package(Eigen3 5.0.0 CONFIG REQUIRED)
    find_package(ltla_sanisizer 0.1.2 CONFIG REQUIRED)
endif()

target_link_libraries(irlba_tatami INTERFACE Eigen3::Eigen tatami::tatami tatami::tatami_mult ltla::irlba ltla::sanisizer)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(IRLBA_TATAMI_TESTS "Build irlba_tatami's test suite." ON)
else()
    option(IRLBA_TATAMI_TESTS "Build irlba_tatami's test suite." OFF)
endif()
if(IRLBA_TATAMI_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

# Install
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libscran)

install(TARGETS irlba_tatami
    EXPORT irlba_tatamiTargets)

install(EXPORT irlba_tatamiTargets
    FILE libscran_irlba_tatamiTargets.cmake
    NAMESPACE libscran::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libscran_irlba_tatami)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/libscran_irlba_tatamiConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libscran_irlba_tatami)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libscran_irlba_tatamiConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libscran_irlba_tatamiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libscran_irlba_tatamiConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libscran_irlba_tatami)
