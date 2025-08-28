// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from clover2_aruco_msgs:msg/MarkerArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "clover2_aruco_msgs/msg/detail/marker_array__rosidl_typesupport_introspection_c.h"
#include "clover2_aruco_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "clover2_aruco_msgs/msg/detail/marker_array__functions.h"
#include "clover2_aruco_msgs/msg/detail/marker_array__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `markers`
#include "clover2_aruco_msgs/msg/marker.h"
// Member `markers`
#include "clover2_aruco_msgs/msg/detail/marker__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  clover2_aruco_msgs__msg__MarkerArray__init(message_memory);
}

void clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_fini_function(void * message_memory)
{
  clover2_aruco_msgs__msg__MarkerArray__fini(message_memory);
}

size_t clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__size_function__MarkerArray__markers(
  const void * untyped_member)
{
  const clover2_aruco_msgs__msg__Marker__Sequence * member =
    (const clover2_aruco_msgs__msg__Marker__Sequence *)(untyped_member);
  return member->size;
}

const void * clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_const_function__MarkerArray__markers(
  const void * untyped_member, size_t index)
{
  const clover2_aruco_msgs__msg__Marker__Sequence * member =
    (const clover2_aruco_msgs__msg__Marker__Sequence *)(untyped_member);
  return &member->data[index];
}

void * clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_function__MarkerArray__markers(
  void * untyped_member, size_t index)
{
  clover2_aruco_msgs__msg__Marker__Sequence * member =
    (clover2_aruco_msgs__msg__Marker__Sequence *)(untyped_member);
  return &member->data[index];
}

void clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__fetch_function__MarkerArray__markers(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const clover2_aruco_msgs__msg__Marker * item =
    ((const clover2_aruco_msgs__msg__Marker *)
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_const_function__MarkerArray__markers(untyped_member, index));
  clover2_aruco_msgs__msg__Marker * value =
    (clover2_aruco_msgs__msg__Marker *)(untyped_value);
  *value = *item;
}

void clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__assign_function__MarkerArray__markers(
  void * untyped_member, size_t index, const void * untyped_value)
{
  clover2_aruco_msgs__msg__Marker * item =
    ((clover2_aruco_msgs__msg__Marker *)
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_function__MarkerArray__markers(untyped_member, index));
  const clover2_aruco_msgs__msg__Marker * value =
    (const clover2_aruco_msgs__msg__Marker *)(untyped_value);
  *item = *value;
}

bool clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__resize_function__MarkerArray__markers(
  void * untyped_member, size_t size)
{
  clover2_aruco_msgs__msg__Marker__Sequence * member =
    (clover2_aruco_msgs__msg__Marker__Sequence *)(untyped_member);
  clover2_aruco_msgs__msg__Marker__Sequence__fini(member);
  return clover2_aruco_msgs__msg__Marker__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_member_array[2] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__MarkerArray, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "markers",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is key
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(clover2_aruco_msgs__msg__MarkerArray, markers),  // bytes offset in struct
    NULL,  // default value
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__size_function__MarkerArray__markers,  // size() function pointer
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_const_function__MarkerArray__markers,  // get_const(index) function pointer
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__get_function__MarkerArray__markers,  // get(index) function pointer
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__fetch_function__MarkerArray__markers,  // fetch(index, &value) function pointer
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__assign_function__MarkerArray__markers,  // assign(index, value) function pointer
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__resize_function__MarkerArray__markers  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_members = {
  "clover2_aruco_msgs__msg",  // message namespace
  "MarkerArray",  // message name
  2,  // number of fields
  sizeof(clover2_aruco_msgs__msg__MarkerArray),
  false,  // has_any_key_member_
  clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_member_array,  // message members
  clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_init_function,  // function to initialize message memory (memory has to be allocated)
  clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_type_support_handle = {
  0,
  &clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_members,
  get_message_typesupport_handle_function,
  &clover2_aruco_msgs__msg__MarkerArray__get_type_hash,
  &clover2_aruco_msgs__msg__MarkerArray__get_type_description,
  &clover2_aruco_msgs__msg__MarkerArray__get_type_description_sources,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_clover2_aruco_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, clover2_aruco_msgs, msg, MarkerArray)() {
  clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, clover2_aruco_msgs, msg, Marker)();
  if (!clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_type_support_handle.typesupport_identifier) {
    clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &clover2_aruco_msgs__msg__MarkerArray__rosidl_typesupport_introspection_c__MarkerArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
