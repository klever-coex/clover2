#include <clover2_fcu_py/capability/all.hpp>
#include <clover2_fcu_py/capability/commands.hpp>
#include <clover2_fcu_py/capability/system_state.hpp>

namespace clover2_fcu_py::capability {

void bind_all(pybind11::object module) {
    bind_system_state(module);
    bind_commands(module);
}

}  // namespace clover2_fcu_py::capability
