// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "clover2_aruco_msgs/msg/detail/marker__rosidl_typesupport_introspection_c.h"
#include "clover2_aruco_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "clover2_aruco_msgs/msg/detail/marker__functions.h"
#include "clover2_aruco_msgs/msg/detail/marker__struct.h"


// Include directives for member types
// Member `pose`
#include "geometry_msgs/msg/pose.h"
// Member `pose`
#include "geometry_msgs/msg/detail/pose__rosidl_typesupport_introspection_c.h"
// Member `c1`
// Member `c2`
// Member `c3`
// Member `c4`
#include "geometry_msgs/msg/point32.h"
// Member `c1`
// Member `c2`
// Member `c3`
// Member `c4`
#include "geometry_msgs/msg/detail/point32__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  clover2_aruco_msgs__msg__Marker__init(message_memory);
}

void clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_fini_function(void * message_memory)
{
  clover2_aruco_msgs__msg__Marker__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[7] = {
  {
    "id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "length",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, length),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "pose",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, pose),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "c1",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, c1),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "c2",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, c2),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "c3",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, c3),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "c4",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__Marker, c4),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_members = {
  "clover2_aruco_msgs__msg",  // message namespace
  "Marker",  // message name
  7,  // number of fields
  sizeof(clover2_aruco_msgs__msg__Marker),
  false,  // has_any_key_member_
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array,  // message members
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_init_function,  // function to initialize message memory (memory has to be allocated)
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_type_support_handle = {
  0,
  &clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_members,
  get_message_typesupport_handle_function,
  &clover2_aruco_msgs__msg__Marker__get_type_hash,
  &clover2_aruco_msgs__msg__Marker__get_type_description,
  &clover2_aruco_msgs__msg__Marker__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_clover2_aruco_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, clover2_aruco_msgs, msg, Marker)() {
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Pose)();
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[3].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Point32)();
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[4].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Point32)();
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[5].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Point32)();
  clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_member_array[6].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, Point32)();
  if (!clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_type_support_handle.typesupport_identifier) {
    clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &clover2_aruco_msgs__msg__Marker__rosidl_typesupport_introspection_c__Marker_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
