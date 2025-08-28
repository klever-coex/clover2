// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from clover2_aruco_msgs:msg/MarkerArray.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker_array.h"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__STRUCT_H_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'markers'
#include "clover2_aruco_msgs/msg/detail/marker__struct.h"

/// Struct defined in msg/MarkerArray in the package clover2_aruco_msgs.
typedef struct clover2_aruco_msgs__msg__MarkerArray
{
  std_msgs__msg__Header header;
  clover2_aruco_msgs__msg__Marker__Sequence markers;
} clover2_aruco_msgs__msg__MarkerArray;

// Struct for a sequence of clover2_aruco_msgs__msg__MarkerArray.
typedef struct clover2_aruco_msgs__msg__MarkerArray__Sequence
{
  clover2_aruco_msgs__msg__MarkerArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} clover2_aruco_msgs__msg__MarkerArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER_ARRAY__STRUCT_H_
