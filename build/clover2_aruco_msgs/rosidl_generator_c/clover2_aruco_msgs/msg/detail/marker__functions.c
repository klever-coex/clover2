// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice
#include "clover2_aruco_msgs/msg/detail/marker__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `pose`
#include "geometry_msgs/msg/detail/pose__functions.h"
// Member `c1`
// Member `c2`
// Member `c3`
// Member `c4`
#include "geometry_msgs/msg/detail/point32__functions.h"

bool
clover2_aruco_msgs__msg__Marker__init(clover2_aruco_msgs__msg__Marker * msg)
{
  if (!msg) {
    return false;
  }
  // id
  // length
  // pose
  if (!geometry_msgs__msg__Pose__init(&msg->pose)) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
    return false;
  }
  // c1
  if (!geometry_msgs__msg__Point32__init(&msg->c1)) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
    return false;
  }
  // c2
  if (!geometry_msgs__msg__Point32__init(&msg->c2)) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
    return false;
  }
  // c3
  if (!geometry_msgs__msg__Point32__init(&msg->c3)) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
    return false;
  }
  // c4
  if (!geometry_msgs__msg__Point32__init(&msg->c4)) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
    return false;
  }
  return true;
}

void
clover2_aruco_msgs__msg__Marker__fini(clover2_aruco_msgs__msg__Marker * msg)
{
  if (!msg) {
    return;
  }
  // id
  // length
  // pose
  geometry_msgs__msg__Pose__fini(&msg->pose);
  // c1
  geometry_msgs__msg__Point32__fini(&msg->c1);
  // c2
  geometry_msgs__msg__Point32__fini(&msg->c2);
  // c3
  geometry_msgs__msg__Point32__fini(&msg->c3);
  // c4
  geometry_msgs__msg__Point32__fini(&msg->c4);
}

bool
clover2_aruco_msgs__msg__Marker__are_equal(const clover2_aruco_msgs__msg__Marker * lhs, const clover2_aruco_msgs__msg__Marker * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // id
  if (lhs->id != rhs->id) {
    return false;
  }
  // length
  if (lhs->length != rhs->length) {
    return false;
  }
  // pose
  if (!geometry_msgs__msg__Pose__are_equal(
      &(lhs->pose), &(rhs->pose)))
  {
    return false;
  }
  // c1
  if (!geometry_msgs__msg__Point32__are_equal(
      &(lhs->c1), &(rhs->c1)))
  {
    return false;
  }
  // c2
  if (!geometry_msgs__msg__Point32__are_equal(
      &(lhs->c2), &(rhs->c2)))
  {
    return false;
  }
  // c3
  if (!geometry_msgs__msg__Point32__are_equal(
      &(lhs->c3), &(rhs->c3)))
  {
    return false;
  }
  // c4
  if (!geometry_msgs__msg__Point32__are_equal(
      &(lhs->c4), &(rhs->c4)))
  {
    return false;
  }
  return true;
}

bool
clover2_aruco_msgs__msg__Marker__copy(
  const clover2_aruco_msgs__msg__Marker * input,
  clover2_aruco_msgs__msg__Marker * output)
{
  if (!input || !output) {
    return false;
  }
  // id
  output->id = input->id;
  // length
  output->length = input->length;
  // pose
  if (!geometry_msgs__msg__Pose__copy(
      &(input->pose), &(output->pose)))
  {
    return false;
  }
  // c1
  if (!geometry_msgs__msg__Point32__copy(
      &(input->c1), &(output->c1)))
  {
    return false;
  }
  // c2
  if (!geometry_msgs__msg__Point32__copy(
      &(input->c2), &(output->c2)))
  {
    return false;
  }
  // c3
  if (!geometry_msgs__msg__Point32__copy(
      &(input->c3), &(output->c3)))
  {
    return false;
  }
  // c4
  if (!geometry_msgs__msg__Point32__copy(
      &(input->c4), &(output->c4)))
  {
    return false;
  }
  return true;
}

clover2_aruco_msgs__msg__Marker *
clover2_aruco_msgs__msg__Marker__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  clover2_aruco_msgs__msg__Marker * msg = (clover2_aruco_msgs__msg__Marker *)allocator.allocate(sizeof(clover2_aruco_msgs__msg__Marker), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(clover2_aruco_msgs__msg__Marker));
  bool success = clover2_aruco_msgs__msg__Marker__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
clover2_aruco_msgs__msg__Marker__destroy(clover2_aruco_msgs__msg__Marker * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    clover2_aruco_msgs__msg__Marker__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
clover2_aruco_msgs__msg__Marker__Sequence__init(clover2_aruco_msgs__msg__Marker__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  clover2_aruco_msgs__msg__Marker * data = NULL;

  if (size) {
    data = (clover2_aruco_msgs__msg__Marker *)allocator.zero_allocate(size, sizeof(clover2_aruco_msgs__msg__Marker), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = clover2_aruco_msgs__msg__Marker__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        clover2_aruco_msgs__msg__Marker__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
clover2_aruco_msgs__msg__Marker__Sequence__fini(clover2_aruco_msgs__msg__Marker__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      clover2_aruco_msgs__msg__Marker__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

clover2_aruco_msgs__msg__Marker__Sequence *
clover2_aruco_msgs__msg__Marker__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  clover2_aruco_msgs__msg__Marker__Sequence * array = (clover2_aruco_msgs__msg__Marker__Sequence *)allocator.allocate(sizeof(clover2_aruco_msgs__msg__Marker__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = clover2_aruco_msgs__msg__Marker__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
clover2_aruco_msgs__msg__Marker__Sequence__destroy(clover2_aruco_msgs__msg__Marker__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    clover2_aruco_msgs__msg__Marker__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
clover2_aruco_msgs__msg__Marker__Sequence__are_equal(const clover2_aruco_msgs__msg__Marker__Sequence * lhs, const clover2_aruco_msgs__msg__Marker__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!clover2_aruco_msgs__msg__Marker__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
clover2_aruco_msgs__msg__Marker__Sequence__copy(
  const clover2_aruco_msgs__msg__Marker__Sequence * input,
  clover2_aruco_msgs__msg__Marker__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(clover2_aruco_msgs__msg__Marker);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    clover2_aruco_msgs__msg__Marker * data =
      (clover2_aruco_msgs__msg__Marker *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!clover2_aruco_msgs__msg__Marker__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          clover2_aruco_msgs__msg__Marker__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!clover2_aruco_msgs__msg__Marker__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
