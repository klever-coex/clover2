// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__rosidl_typesupport_fastrtps_cpp.hpp.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_

#include <cstddef>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "clover2_aruco_msgs/msg/rosidl_typesupport_fastrtps_cpp__visibility_control.h"
#include "clover2_aruco_msgs/msg/detail/marker__struct.hpp"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "fastcdr/Cdr.h"

namespace clover2_aruco_msgs
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
cdr_serialize(
  const clover2_aruco_msgs::msg::Marker & ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  clover2_aruco_msgs::msg::Marker & ros_message);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
get_serialized_size(
  const clover2_aruco_msgs::msg::Marker & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
max_serialized_size_Marker(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
cdr_serialize_key(
  const clover2_aruco_msgs::msg::Marker & ros_message,
  eprosima::fastcdr::Cdr &);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
get_serialized_size_key(
  const clover2_aruco_msgs::msg::Marker & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
max_serialized_size_key_Marker(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace clover2_aruco_msgs

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_clover2_aruco_msgs
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, clover2_aruco_msgs, msg, Marker)();

#ifdef __cplusplus
}
#endif

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
