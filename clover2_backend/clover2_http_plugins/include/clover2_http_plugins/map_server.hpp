#pragma once

// clover2
#include <clover2_http/plugin.hpp>
#include <clover2_http_plugins/data/map_info.hpp>
#include <clover2_http_plugins/data/marker_info.hpp>
#include <clover2_http_plugins/data/modify_result.hpp>
#include <clover2_map/client.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

namespace clover2_http_plugins {

// REST plugin exposing the clover2_map::client functionality:
//   GET    /api/map                      - the cached map
//   GET    /api/map/marker/-/{id}        - one marker
//   POST   /api/map/marker               - add a marker
//   PUT    /api/map/marker/-/{id}        - edit a marker
//   DELETE /api/map/marker/-/{id}        - delete a marker
class map_server : public clover2_http::plugin<map_server> {
public:
    static constexpr std::string_view k_name = "map_server";
    static constexpr int k_version = 1;

protected:
    void on_initialize() override;
    std::vector<std::string> capabilities() const override;

private:
    void handle_map(clover2_http::http::core::request_context ctx,
                    clover2_http::http::endpoint::deferred_reply<
                        clover2_http_plugins::data::map_info> reply);

    void handle_marker(clover2_http::http::core::request_context ctx,
                       clover2_http::http::endpoint::deferred_reply<
                           clover2_http_plugins::data::marker_info> reply);

    void handle_add(clover2_http::http::core::request_context ctx,
                    clover2_http_plugins::data::marker_info request,
                    clover2_http::http::endpoint::deferred_reply<
                        clover2_http_plugins::data::modify_result> reply);

    void handle_edit(clover2_http::http::core::request_context ctx,
                     clover2_http_plugins::data::marker_info request,
                     clover2_http::http::endpoint::deferred_reply<
                         clover2_http_plugins::data::modify_result> reply);

    void handle_delete(clover2_http::http::core::request_context ctx,
                       clover2_http::http::endpoint::deferred_reply<
                           clover2_http_plugins::data::modify_result> reply);

    std::shared_ptr<clover2_map::client> m_map_client;
    rclcpp::CallbackGroup::SharedPtr m_map_group;
    rclcpp::TimerBase::SharedPtr m_refresh_timer;
};

}  // namespace clover2_http_plugins
