#pragma once

// clover2
#include <clover2_common/node_runtime.hpp>
#include <clover2_fcu/client.hpp>
#include <clover2_fcu_py/settings.hpp>

// pybind
#include <pybind11/pybind11.h>

// STL
#include <memory>
#include <string>

namespace clover2_fcu_py {

class client {
public:
    client();
    explicit client(const settings& s);
    ~client();

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    std::string node_name() const;

    bool has(const std::string& capability);

private:
    friend void bind_client(pybind11::object module);

    pybind11::object getitem(const std::string& capability);

    settings m_settings;
    std::shared_ptr<clover2_common::node_runtime> m_runtime;
    clover2_fcu::client::shared_ptr m_client;
};

void bind_client(pybind11::object module);

}  // namespace clover2_fcu_py
