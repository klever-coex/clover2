#include <clover2_common/util/parameter.hpp>
#include <clover2_led/device/base_device.hpp>

#include <gz/msgs/float_v.pb.h>
#include <gz/transport/Node.hh>

#include <pluginlib/class_list_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace clover2_led::device {

class gz_led_strip : public base_device {
protected:
    void on_initialize(size_t led_count) override {
        auto parameters_interface =
            get_node_context()->get_node_parameters_interface();

        const auto parameter_prefix = get_name() + ".";
        clover2_common::util::declare_parameter_if_not_declared(
            parameters_interface, parameter_prefix + "topic",
            std::string("/model/klever5/led_strip/frame"));
        clover2_common::util::declare_parameter_if_not_declared(
            parameters_interface, parameter_prefix + "max_fps", 30.0);

        rclcpp::Parameter topic_parameter;
        rclcpp::Parameter max_fps_parameter;
        parameters_interface->get_parameter(parameter_prefix + "topic",
                                             topic_parameter);
        parameters_interface->get_parameter(parameter_prefix + "max_fps",
                                             max_fps_parameter);

        m_topic = topic_parameter.as_string();
        info().max_fps = std::max(1.0, max_fps_parameter.as_double());
        m_publisher = m_transport_node.Advertise<gz::msgs::Float_V>(m_topic);

        RCLCPP_INFO(get_logger(),
                    "gz_led_strip: publishing %zu LEDs to %s at %.1f FPS",
                    led_count, m_topic.c_str(), info().max_fps);
    }

    void on_cleanup() override { m_publisher = {}; }

    void write_raw_frame(
        const std::vector<clover2_led::data::color>& colors) override {
        gz::msgs::Float_V message;
        message.mutable_data()->Reserve(static_cast<int>(colors.size() * 3));

        for (const auto& color : colors) {
            message.add_data(static_cast<float>(color.r) / 255.0F);
            message.add_data(static_cast<float>(color.g) / 255.0F);
            message.add_data(static_cast<float>(color.b) / 255.0F);
        }

        m_publisher.Publish(message);
    }

private:
    gz::transport::Node m_transport_node;
    gz::transport::Node::Publisher m_publisher;
    std::string m_topic;
};

}  // namespace clover2_led::device

PLUGINLIB_EXPORT_CLASS(clover2_led::device::gz_led_strip,
                       clover2_led::device::base_device)