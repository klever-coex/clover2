#pragma once

#include <rclcpp/serialized_message.hpp>
#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>

#include <memory>
#include <string>

namespace rcpputils {
class SharedLibrary;
}

namespace clover2_http::plugins::utils {

class message_type {
public:
    explicit message_type(const std::string& type_name);
    ~message_type();

    message_type(const message_type&) = delete;
    message_type& operator=(const message_type&) = delete;

    void* allocate();
    void deallocate(void* msg);

    bool serialize(const void* msg, rclcpp::SerializedMessage& out);
    bool deserialize(const rclcpp::SerializedMessage& message, void* msg);

    const rosidl_typesupport_introspection_cpp::MessageMembers* members()
        const {
        return m_members;
    }

private:
    std::shared_ptr<rcpputils::SharedLibrary> m_introspection_lib;
    std::shared_ptr<rcpputils::SharedLibrary> m_rmw_lib;
    const rosidl_typesupport_introspection_cpp::MessageMembers* m_members =
        nullptr;
    const rosidl_message_type_support_t* m_rmw_handle = nullptr;
};

}  // namespace clover2_http::plugins::utils
