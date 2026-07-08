#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>
#include <rclcpp/node.hpp>

namespace clover2_common::node_interfaces {

class NodeParametersWatcher::NodeParametersWatcherImpl {
public:
    NodeParametersWatcherImpl(
        const rclcpp::node_interfaces::NodeParametersInterface::SharedPtr&
            node_parameters)
        : m_param_interface(node_parameters) {
        m_set_parameters_handle =
            m_param_interface->add_on_set_parameters_callback(
                std::bind(&NodeParametersWatcherImpl::on_set_parameters_cb,
                          this, std::placeholders::_1));
    }

    ~NodeParametersWatcherImpl() {
        undeclare_watcher_parameters();

        m_param_interface->remove_on_set_parameters_callback(
            m_set_parameters_handle.get());
    }

    void declare_and_watch_parameter(
        const std::string& name, const rclcpp::ParameterValue& default_value,
        ParameterFunctorT cb,
        const rcl_interfaces::msg::ParameterDescriptor& parameter_descriptor =
            rcl_interfaces::msg::ParameterDescriptor(),
        bool ignore_override = false) {
        m_watch_parameters[name] = cb;

        m_param_interface->declare_parameter(
            name, default_value, parameter_descriptor, ignore_override);
    }

    void undeclare_watcher_parameters() {
        for (const auto& [name, callback] : m_watch_parameters) {
            if (m_param_interface->has_parameter(name)) {
                try {
                    m_param_interface->undeclare_parameter(name);
                } catch (...) {
                }
            }
        }

        m_watch_parameters.clear();
    }

private:
    rcl_interfaces::msg::SetParametersResult on_set_parameters_cb(
        const std::vector<rclcpp::Parameter>& parameters) {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;

        for (auto& p : parameters) {
            auto it = m_watch_parameters.find(p.get_name());
            if (it != m_watch_parameters.end()) {
                try {
                    it->second(p);
                } catch (std::exception& ex) {
                    result.successful = false;
                    result.reason = ex.what();
                    break;
                }
            }
        }

        return result;
    }

    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        m_param_interface;
    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle;
    std::unordered_map<std::string, NodeParametersWatcher::ParameterFunctorT>
        m_watch_parameters;
};

NodeParametersWatcher::NodeParametersWatcher(
    const rclcpp::node_interfaces::NodeParametersInterface::SharedPtr&
        node_parameters)
    : m_impl(new NodeParametersWatcherImpl(node_parameters)) {}

NodeParametersWatcher::~NodeParametersWatcher() {}

void NodeParametersWatcher::declare_and_watch_parameter(
    const std::string& name, const rclcpp::ParameterValue& default_value,
    ParameterFunctorT cb,
    const rcl_interfaces::msg::ParameterDescriptor& parameter_descriptor,
    bool ignore_override) {
    m_impl->declare_and_watch_parameter(name, default_value, cb,
                                        parameter_descriptor, ignore_override);
}

void NodeParametersWatcher::undeclare_watcher_parameters() {
    m_impl->undeclare_watcher_parameters();
}

}  // namespace clover2_common::node_interfaces
