cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)

FetchContent_Declare(
  tatami
  GIT_REPOSITORY https://github.com/tatami-inc/tatami
  GIT_TAG master # ^4.0.0
)

FetchContent_Declare(
  tatami_mult
  GIT_REPOSITORY https://github.com/tatami-inc/tatami_mult
  GIT_TAG master # ^0.1.0
)

FetchContent_Declare(
  eigen 
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen
  GIT_TAG 5.0.0
)

FetchContent_Declare(
  irlba
  GIT_REPOSITORY https://github.com/LTLA/CppIrlba
  GIT_TAG master # ^3.0.0
)

FetchContent_Declare(
  sanisizer
  GIT_REPOSITORY https://github.com/LTLA/sanisizer
  GIT_TAG master # ^0.1.3
)

# Forcing the Eigen build to create the installation files,
# otherwise irlba's own installation gets confused.
option(EIGEN_BUILD_CMAKE_PACKAGE "Build the Eigen Cmake package" ON)
FetchContent_MakeAvailable(eigen)

FetchContent_MakeAvailable(irlba)
FetchContent_MakeAvailable(tatami)
FetchContent_MakeAvailable(tatami_mult)
FetchContent_MakeAvailable(sanisizer)
