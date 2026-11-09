#pragma once

// clover2
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/logger.hpp>

// msgs
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/set_mode.hpp>

// STL
#include <memory>
#include <optional>
#include <string>

namespace clover2_fcu_mavros::capability {

using clover2_fcu::data::flight_mode;

class commands : public clover2_fcu::capability::commands_interface,
                 public std::enable_shared_from_this<commands> {
public:
    explicit commands(const clover2_fcu::context& ctx, std::string prefix);

    void arm(result_callback cb, bool run_preflight_checks) override;
    void disarm(result_callback cb) override;
    void set_mode(result_callback cb, flight_mode mode) override;
    void takeoff(result_callback cb, std::optional<double> altitude,
                 std::optional<double> heading) override;
    void land(result_callback cb) override;
    void rtl(result_callback cb) override;

private:
    template <typename ServiceT, typename Check>
    void invoke(const std::string& name,
                const typename rclcpp::Client<ServiceT>::SharedPtr& client,
                std::shared_ptr<typename ServiceT::Request> request,
                result_callback cb, Check ack) {
        if (!client->service_is_ready()) {
            RCLCPP_WARN(m_logger, "command '%s': service '%s' is not ready",
                        name.c_str(), client->get_service_name());
            cb(false);
            return;
        }

        std::weak_ptr<commands> self = weak_from_this();
        client->async_send_request(
            request,
            [self = std::move(self), name, cb = std::move(cb),
             ack = std::move(ack), logger = m_logger](
                typename rclcpp::Client<ServiceT>::SharedFuture future) {
                const bool ok = ack(*future.get());
                RCLCPP_DEBUG(logger, "command '%s' ack: %s", name.c_str(),
                             ok ? "accepted" : "rejected");
                if (self.lock()) {
                    cb(ok);
                }
            });
    }

    rclcpp::Logger m_logger;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr m_arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr m_set_mode_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr m_takeoff_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr m_land_client;
};

}  // namespace clover2_fcu_mavros::capability
