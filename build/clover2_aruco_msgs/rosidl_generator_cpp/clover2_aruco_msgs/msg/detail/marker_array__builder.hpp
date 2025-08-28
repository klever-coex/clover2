// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from clover2_aruco_msgs:msg/MarkerArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker_array.hpp"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__BUILDER_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "clover2_aruco_msgs/msg/detail/marker_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace clover2_aruco_msgs
{

namespace msg
{

namespace builder
{

class Init_MarkerArray_markers
{
public:
  explicit Init_MarkerArray_markers(::clover2_aruco_msgs::msg::MarkerArray & msg)
  : msg_(msg)
  {}
  ::clover2_aruco_msgs::msg::MarkerArray markers(::clover2_aruco_msgs::msg::MarkerArray::_markers_type arg)
  {
    msg_.markers = std::move(arg);
    return std::move(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::MarkerArray msg_;
};

class Init_MarkerArray_header
{
public:
  Init_MarkerArray_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_MarkerArray_markers header(::clover2_aruco_msgs::msg::MarkerArray::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_MarkerArray_markers(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::MarkerArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::clover2_aruco_msgs::msg::MarkerArray>()
{
  return clover2_aruco_msgs::msg::builder::Init_MarkerArray_header();
}

}  // namespace clover2_aruco_msgs

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__BUILDER_HPP_
