#include <pybind11/pybind11.h>
#include "humanoid_sdk.h"

namespace py = pybind11;


PYBIND11_MODULE(py_humanoid_sdk, m) {
    m.doc() = "humanoid_sdk python wrapper.";

}
