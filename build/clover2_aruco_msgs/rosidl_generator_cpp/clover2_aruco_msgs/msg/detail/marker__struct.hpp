// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "clover2_aruco_msgs/msg/marker.hpp"


#ifndef CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_HPP_
#define CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__struct.hpp"
// Member 'c1'
// Member 'c2'
// Member 'c3'
// Member 'c4'
#include "geometry_msgs/msg/detail/point32__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__clover2_aruco_msgs__msg__Marker __attribute__((deprecated))
#else
# define DEPRECATED__clover2_aruco_msgs__msg__Marker __declspec(deprecated)
#endif

namespace clover2_aruco_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Marker_
{
  using Type = Marker_<ContainerAllocator>;

  explicit Marker_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : pose(_init),
    c1(_init),
    c2(_init),
    c3(_init),
    c4(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0ul;
      this->length = 0.0f;
    }
  }

  explicit Marker_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : pose(_alloc, _init),
    c1(_alloc, _init),
    c2(_alloc, _init),
    c3(_alloc, _init),
    c4(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0ul;
      this->length = 0.0f;
    }
  }

  // field types and members
  using _id_type =
    uint32_t;
  _id_type id;
  using _length_type =
    float;
  _length_type length;
  using _pose_type =
    geometry_msgs::msg::Pose_<ContainerAllocator>;
  _pose_type pose;
  using _c1_type =
    geometry_msgs::msg::Point32_<ContainerAllocator>;
  _c1_type c1;
  using _c2_type =
    geometry_msgs::msg::Point32_<ContainerAllocator>;
  _c2_type c2;
  using _c3_type =
    geometry_msgs::msg::Point32_<ContainerAllocator>;
  _c3_type c3;
  using _c4_type =
    geometry_msgs::msg::Point32_<ContainerAllocator>;
  _c4_type c4;

  // setters for named parameter idiom
  Type & set__id(
    const uint32_t & _arg)
  {
    this->id = _arg;
    return *this;
  }
  Type & set__length(
    const float & _arg)
  {
    this->length = _arg;
    return *this;
  }
  Type & set__pose(
    const geometry_msgs::msg::Pose_<ContainerAllocator> & _arg)
  {
    this->pose = _arg;
    return *this;
  }
  Type & set__c1(
    const geometry_msgs::msg::Point32_<ContainerAllocator> & _arg)
  {
    this->c1 = _arg;
    return *this;
  }
  Type & set__c2(
    const geometry_msgs::msg::Point32_<ContainerAllocator> & _arg)
  {
    this->c2 = _arg;
    return *this;
  }
  Type & set__c3(
    const geometry_msgs::msg::Point32_<ContainerAllocator> & _arg)
  {
    this->c3 = _arg;
    return *this;
  }
  Type & set__c4(
    const geometry_msgs::msg::Point32_<ContainerAllocator> & _arg)
  {
    this->c4 = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    clover2_aruco_msgs::msg::Marker_<ContainerAllocator> *;
  using ConstRawPtr =
    const clover2_aruco_msgs::msg::Marker_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      clover2_aruco_msgs::msg::Marker_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      clover2_aruco_msgs::msg::Marker_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__clover2_aruco_msgs__msg__Marker
    std::shared_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__clover2_aruco_msgs__msg__Marker
    std::shared_ptr<clover2_aruco_msgs::msg::Marker_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Marker_ & other) const
  {
    if (this->id != other.id) {
      return false;
    }
    if (this->length != other.length) {
      return false;
    }
    if (this->pose != other.pose) {
      return false;
    }
    if (this->c1 != other.c1) {
      return false;
    }
    if (this->c2 != other.c2) {
      return false;
    }
    if (this->c3 != other.c3) {
      return false;
    }
    if (this->c4 != other.c4) {
      return false;
    }
    return true;
  }
  bool operator!=(const Marker_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Marker_

// alias to use template instance with default allocator
using Marker =
  clover2_aruco_msgs::msg::Marker_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace clover2_aruco_msgs

#endif  // CLOVER2_ARUCO_MSGS__MSG__DETAIL__MARKER__STRUCT_HPP_
