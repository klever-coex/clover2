#pragma once

#include <nlohmann/json.hpp>

#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>
#include <rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp>

#include <stdexcept>

namespace clover2_http_plugins::utils::msg_json {

template <typename MessageT>
nlohmann::json to_json(const MessageT& msg);

template <typename MessageT>
void from_json(const nlohmann::json& j, MessageT& msg);

namespace detail {

template <typename MessageT>
const rosidl_typesupport_introspection_cpp::MessageMembers* members_of() {
    const auto* ts =
        rosidl_typesupport_introspection_cpp::get_message_type_support_handle<
            MessageT>();
    if (!ts) {
        throw std::runtime_error(
            "introspection typesupport is not available for the message "
            "type; link <pkg>__rosidl_typesupport_introspection_cpp");
    }

    return static_cast<
        const rosidl_typesupport_introspection_cpp::MessageMembers*>(
        ts->data);
}

nlohmann::json to_json(
    const rosidl_typesupport_introspection_cpp::MessageMembers* members,
    const void* msg);

void from_json(
    const rosidl_typesupport_introspection_cpp::MessageMembers* members,
    void* msg, const nlohmann::json& j);

}  // namespace detail

template <typename MessageT>
nlohmann::json to_json(const MessageT& msg) {
    return detail::to_json(detail::members_of<MessageT>(), &msg);
}

template <typename MessageT>
void from_json(const nlohmann::json& j, MessageT& msg) {
    detail::from_json(detail::members_of<MessageT>(), &msg, j);
}

}  // namespace clover2_http_plugins::utils::msg_json
