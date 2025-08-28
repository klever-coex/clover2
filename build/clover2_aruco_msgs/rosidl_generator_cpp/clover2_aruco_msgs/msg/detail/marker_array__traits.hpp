// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from clover2_aruco_msgs:msg/MarkerArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker_array.hpp"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__TRAITS_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "clover2_aruco_msgs/msg/detail/marker_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'markers'
#include "clover2_aruco_msgs/msg/detail/marker__traits.hpp"

namespace clover2_aruco_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const MarkerArray & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: markers
  {
    if (msg.markers.size() == 0) {
      out << "markers: []";
    } else {
      out << "markers: [";
      size_t pending_items = msg.markers.size();
      for (auto item : msg.markers) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const MarkerArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: markers
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.markers.size() == 0) {
      out << "markers: []\n";
    } else {
      out << "markers:\n";
      for (auto item : msg.markers) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const MarkerArray & msg, bool use_flow_style = false)
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
  const clover2_aruco_msgs::msg::MarkerArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  clover2_aruco_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use clover2_aruco_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const clover2_aruco_msgs::msg::MarkerArray & msg)
{
  return clover2_aruco_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<clover2_aruco_msgs::msg::MarkerArray>()
{
  return "clover2_aruco_msgs::msg::MarkerArray";
}

template<>
inline const char * name<clover2_aruco_msgs::msg::MarkerArray>()
{
  return "clover2_aruco_msgs/msg/MarkerArray";
}

template<>
struct has_fixed_size<clover2_aruco_msgs::msg::MarkerArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<clover2_aruco_msgs::msg::MarkerArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<clover2_aruco_msgs::msg::MarkerArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__TRAITS_HPP_
