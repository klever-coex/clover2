// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker.h"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_H_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__struct.h"
// Member 'c1'
// Member 'c2'
// Member 'c3'
// Member 'c4'
#include "geometry_msgs/msg/detail/point32__struct.h"

/// Struct defined in msg/Marker in the package clover2_aruco_msgs.
typedef struct clover2_aruco_msgs__msg__Marker
{
  /// Unique marker identificator
  uint32_t id;
  /// Side size in meters
  float length;
  /// Marker position
  geometry_msgs__msg__Pose pose;
  /// Z field unused
  geometry_msgs__msg__Point32 c1;
  geometry_msgs__msg__Point32 c2;
  geometry_msgs__msg__Point32 c3;
  geometry_msgs__msg__Point32 c4;
} clover2_aruco_msgs__msg__Marker;

// Struct for a sequence of clover2_aruco_msgs__msg__Marker.
typedef struct clover2_aruco_msgs__msg__Marker__Sequence
{
  clover2_aruco_msgs__msg__Marker * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} clover2_aruco_msgs__msg__Marker__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_H_
