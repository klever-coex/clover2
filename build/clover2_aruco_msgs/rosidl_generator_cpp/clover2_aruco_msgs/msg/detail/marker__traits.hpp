// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker.hpp"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__TRAITS_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "clover2_aruco_msgs/msg/detail/marker__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__traits.hpp"
// Member 'c1'
// Member 'c2'
// Member 'c3'
// Member 'c4'
#include "geometry_msgs/msg/detail/point32__traits.hpp"

namespace clover2_aruco_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const Marker & msg,
  std::ostream & out)
{
  out << "{";
  // member: id
  {
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << ", ";
  }

  // member: length
  {
    out << "length: ";
    rosidl_generator_traits::value_to_yaml(msg.length, out);
    out << ", ";
  }

  // member: pose
  {
    out << "pose: ";
    to_flow_style_yaml(msg.pose, out);
    out << ", ";
  }

  // member: c1
  {
    out << "c1: ";
    to_flow_style_yaml(msg.c1, out);
    out << ", ";
  }

  // member: c2
  {
    out << "c2: ";
    to_flow_style_yaml(msg.c2, out);
    out << ", ";
  }

  // member: c3
  {
    out << "c3: ";
    to_flow_style_yaml(msg.c3, out);
    out << ", ";
  }

  // member: c4
  {
    out << "c4: ";
    to_flow_style_yaml(msg.c4, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Marker & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << "\n";
  }

  // member: length
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "length: ";
    rosidl_generator_traits::value_to_yaml(msg.length, out);
    out << "\n";
  }

  // member: pose
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pose:\n";
    to_block_style_yaml(msg.pose, out, indentation + 2);
  }

  // member: c1
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "c1:\n";
    to_block_style_yaml(msg.c1, out, indentation + 2);
  }

  // member: c2
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "c2:\n";
    to_block_style_yaml(msg.c2, out, indentation + 2);
  }

  // member: c3
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "c3:\n";
    to_block_style_yaml(msg.c3, out, indentation + 2);
  }

  // member: c4
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "c4:\n";
    to_block_style_yaml(msg.c4, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Marker & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace clover2_aruco_msgs

namespace rosidl_generator_traits
{

[[deprecated("use clover2_aruco_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const clover2_aruco_msgs::msg::Marker & msg,
  std::ostream & out, size_t indentation = 0)
{
  clover2_aruco_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use clover2_aruco_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const clover2_aruco_msgs::msg::Marker & msg)
{
  return clover2_aruco_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<clover2_aruco_msgs::msg::Marker>()
{
  return "clover2_aruco_msgs::msg::Marker";
}

template<>
inline const char * name<clover2_aruco_msgs::msg::Marker>()
{
  return "clover2_aruco_msgs/msg/Marker";
}

template<>
struct has_fixed_size<clover2_aruco_msgs::msg::Marker>
  : std::integral_constant<bool, has_fixed_size<geometry_msgs::msg::Point32>::value && has_fixed_size<geometry_msgs::msg::Pose>::value> {};

template<>
struct has_bounded_size<clover2_aruco_msgs::msg::Marker>
  : std::integral_constant<bool, has_bounded_size<geometry_msgs::msg::Point32>::value && has_bounded_size<geometry_msgs::msg::Pose>::value> {};

template<>
struct is_message<clover2_aruco_msgs::msg::Marker>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__TRAITS_HPP_
