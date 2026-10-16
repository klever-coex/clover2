#include <clover2_http/plugins/utils/message_type.hpp>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <rclcpp/typesupport_helpers.hpp>
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <rosidl_typesupport_fastrtps_cpp/message_type_support.h>

#include <cstdio>
#include <stdexcept>

namespace clover2_http::plugins::utils {

message_type::message_type(const std::string& type_name) {
    m_introspection_lib = rclcpp::get_typesupport_library(
        type_name, "rosidl_typesupport_introspection_cpp");
    m_rmw_lib = rclcpp::get_typesupport_library(
        type_name, "rosidl_typesupport_fastrtps_cpp");

    const auto* introspection_handle = rclcpp::get_message_typesupport_handle(
        type_name, "rosidl_typesupport_introspection_cpp",
        *m_introspection_lib);
    m_rmw_handle = rclcpp::get_message_typesupport_handle(
        type_name, "rosidl_typesupport_fastrtps_cpp", *m_rmw_lib);

    m_members = static_cast<
        const rosidl_typesupport_introspection_cpp::MessageMembers*>(
        introspection_handle->data);
}

message_type::~message_type() = default;

void* message_type::allocate() {
    void* msg = ::operator new(m_members->size_of_);
    m_members->init_function(msg,
                             rosidl_runtime_cpp::MessageInitialization::ALL);
    return msg;
}

void message_type::deallocate(void* msg) {
    m_members->fini_function(msg);
    ::operator delete(msg);
}

const message_type_support_callbacks_t* callbacks_of(
    const rosidl_message_type_support_t* handle) {
    return static_cast<const message_type_support_callbacks_t*>(handle->data);
}

bool message_type::serialize(const void* msg, rclcpp::SerializedMessage& out) {
    const auto* callbacks = callbacks_of(m_rmw_handle);

    char bounds_info = 0;
    size_t capacity = callbacks->max_serialized_size(bounds_info);
    if (capacity < 256) {
        capacity = 256;
    }

    for (;;) {
        out.reserve(capacity);
        auto& rcl_msg = out.get_rcl_serialized_message();
        eprosima::fastcdr::FastBuffer buffer(
            reinterpret_cast<char*>(rcl_msg.buffer), rcl_msg.buffer_capacity);
        eprosima::fastcdr::Cdr cdr(buffer);

        try {
            cdr.serialize_encapsulation();
            if (!callbacks->cdr_serialize(msg, cdr)) {
                throw std::runtime_error(
                    "typesupport: CDR serialization failed");
            }

            rcl_msg.buffer_length = cdr.get_serialized_data_length();

            return true;
        } catch (
            const eprosima::fastcdr::exception::NotEnoughMemoryException&) {
            capacity *= 2;
        }
    }
}

bool message_type::deserialize(const rclcpp::SerializedMessage& message,
                               void* msg) {
    const auto* callbacks = callbacks_of(m_rmw_handle);

    const auto& rcl_msg = message.get_rcl_serialized_message();
    const size_t length = rcl_msg.buffer_length;
    char* data = reinterpret_cast<char*>(rcl_msg.buffer);

    eprosima::fastcdr::FastBuffer buffer(data, length);
    eprosima::fastcdr::Cdr cdr(buffer);

    try {
        cdr.read_encapsulation();

        if (!callbacks->cdr_deserialize(cdr, msg)) {
            throw std::runtime_error("typesupport: CDR deserialization failed");
        }
    } catch (const std::exception& e) {
        std::string head;
        const auto* raw =
            reinterpret_cast<const unsigned char*>(rcl_msg.buffer);

        for (size_t i = 0; i < rcl_msg.buffer_length && i < 32; ++i) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%02x ", raw[i]);
            head += buf;
        }

        throw std::runtime_error(
            "typesupport: deserialize failed: " + std::string(e.what()) +
            ", len=" + std::to_string(rcl_msg.buffer_length) +
            ", head: " + head);
    }

    return true;
}

}  // namespace clover2_http::plugins::utils
