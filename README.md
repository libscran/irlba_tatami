# tatami bindings for irlba

![Unit tests](https://github.com/libscran/irlba_tatami/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/libscran/irlba_tatami/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/libscran/irlba_tatami/branch/master/graph/badge.svg?token=dvjUvDnMEL)](https://codecov.io/gh/libscran/irlba_tatami)

## Overview

This library implements a wrapper class to use [`tatami::Matrix`](https://github.com/tatami-inc/tatami) instances in the [**irlba** library](https://github.com/libscran/irlba).
The goal is to support IRLBA on alternative matrix representations (e.g., sparse, file-backed) without requiring realization into a `irlba::SimpleMatrix`.

## Quick start

Not much to say, really. 
Just pass a `irlba_tatami::Normal` or `irlba_tatami::Transposed` anywhere that an `irlba::Matrix` can be accepted:

```cpp
#include "irlba_tatami/irlba_tatami.hpp"

// Initialize this with an instance of a concrete tatami subclass.
std::shared_ptr<tatami::Matrix<double, int> > tmat;

// Wrap it for use in IRLBA.
irlba_tatami::Normal<Eigen::VectorXd, Eigen::MatrixXd, double, int> wrapped(std::move(tmat));
auto res = irlba::compute(wrapper, 5, irlba::Options());

// Performing IRLBA on a column-centered matrix.
Eigen::VectorXd centers; // Fill column centers here...
CenteredMatrix<Eigen::VectorXd, Eigen::MatrixXd> centered(&wrapped, &centers);
auto centered_res = irlba::compute(centered, 5, irlba::Options());
```

See the [reference documentation](https://libscran.github.io/irlba_tatami) for more details.

## Building projects 

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  irlba_tatami
  GIT_REPOSITORY https://github.com/libscran/irlba_tatami
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(irlba_tatami)
```

Then you can link to **irlba_tatami** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe libscran::irlba_tatami)

# For libaries
target_link_libraries(mylib INTERFACE libscran::irlba_tatami)
```

By default, this will use `FetchContent` to fetch all external dependencies. 
Applications are advised to pin the versions of each dependency for stability - see [`extern/CMakeLists.txt`](extern/CMakeLists.txt) for suggested versions.
If you want to install them manually, use `-DIRLBA_TATAMI_FETCH_EXTERN=OFF`.

### CMake with `find_package()`

To install the library, clone an appropriate version of this repository and run:

```sh
mkdir build && cd build
cmake .. -DIRLBA_TATAMI_TESTS=OFF
cmake --build . --target install
```

Then we can use `find_package()` as usual:

```cmake
find_package(libscran_irlba_tatami CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE libscran::irlba_tatami)
```

Again, this will automatically acquire all its dependencies, see recommendations above.

### Manual

If you're not using CMake, the simple approach is to just copy the files in `include/` - either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
This requires the external dependencies listed in [`extern/CMakeLists.txt`](extern/CMakeLists.txt). 
