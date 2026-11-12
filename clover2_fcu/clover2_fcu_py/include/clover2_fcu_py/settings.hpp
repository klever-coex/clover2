#pragma once

// clover2
#include <clover2_fcu/client.hpp>

// pybind
#include <pybind11/pybind11.h>

// STL
#include <string>
#include <vector>

namespace clover2_fcu_py {

class settings {
public:
    settings() = default;

    bool get_use_diagnostics() const;
    void set_use_diagnostics(bool val);

    const std::string& get_bridge_node() const;
    void set_bridge_node(const std::string& val);

    double get_wait_timeout() const;
    void set_wait_timeout(double seconds);

    std::vector<std::string> get_required() const;

    void require(const std::string& name);

    /// @brief The wrapped C++ settings (the client binding reads them).
    const clover2_fcu::client::settings& raw() const { return m_settings; }

private:
    clover2_fcu::client::settings m_settings;
};

void bind_settings(pybind11::object module);

}  // namespace clover2_fcu_py
