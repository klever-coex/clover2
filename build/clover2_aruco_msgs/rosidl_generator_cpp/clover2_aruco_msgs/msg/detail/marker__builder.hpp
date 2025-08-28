// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker.hpp"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__BUILDER_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "clover2_aruco_msgs/msg/detail/marker__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace clover2_aruco_msgs
{

namespace msg
{

namespace builder
{

class Init_Marker_c4
{
public:
  explicit Init_Marker_c4(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  ::clover2_aruco_msgs::msg::Marker c4(::clover2_aruco_msgs::msg::Marker::_c4_type arg)
  {
    msg_.c4 = std::move(arg);
    return std::move(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_c3
{
public:
  explicit Init_Marker_c3(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  Init_Marker_c4 c3(::clover2_aruco_msgs::msg::Marker::_c3_type arg)
  {
    msg_.c3 = std::move(arg);
    return Init_Marker_c4(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_c2
{
public:
  explicit Init_Marker_c2(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  Init_Marker_c3 c2(::clover2_aruco_msgs::msg::Marker::_c2_type arg)
  {
    msg_.c2 = std::move(arg);
    return Init_Marker_c3(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_c1
{
public:
  explicit Init_Marker_c1(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  Init_Marker_c2 c1(::clover2_aruco_msgs::msg::Marker::_c1_type arg)
  {
    msg_.c1 = std::move(arg);
    return Init_Marker_c2(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_pose
{
public:
  explicit Init_Marker_pose(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  Init_Marker_c1 pose(::clover2_aruco_msgs::msg::Marker::_pose_type arg)
  {
    msg_.pose = std::move(arg);
    return Init_Marker_c1(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_length
{
public:
  explicit Init_Marker_length(::clover2_aruco_msgs::msg::Marker & msg)
  : msg_(msg)
  {}
  Init_Marker_pose length(::clover2_aruco_msgs::msg::Marker::_length_type arg)
  {
    msg_.length = std::move(arg);
    return Init_Marker_pose(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

class Init_Marker_id
{
public:
  Init_Marker_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Marker_length id(::clover2_aruco_msgs::msg::Marker::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_Marker_length(msg_);
  }

private:
  ::clover2_aruco_msgs::msg::Marker msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::clover2_aruco_msgs::msg::Marker>()
{
  return clover2_aruco_msgs::msg::builder::Init_Marker_id();
}

}  // namespace clover2_aruco_msgs

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__BUILDER_HPP_
