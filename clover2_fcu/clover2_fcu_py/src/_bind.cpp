#include <clover2_fcu_py/client.hpp>
#include <clover2_fcu_py/settings.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

PYBIND11_MODULE(clover2_fcu_py, m) {
    m.doc() = "Python bindings for clover2_fcu.";

    clover2_fcu_py::bind_settings(m);
    clover2_fcu_py::bind_client(m);
}
