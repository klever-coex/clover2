// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from clover2_aruco_msgs:msg/Marker.idl
// generated code does not contain a copyright notice
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <stdbool.h>
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "numpy/ndarrayobject.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
#include "rosidl_runtime_c/visibility_control.h"
#include "clover2_aruco_msgs/msg/detail/marker__struct.h"
#include "clover2_aruco_msgs/msg/detail/marker__functions.h"

ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__pose__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__pose__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__point32__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__point32__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__point32__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__point32__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__point32__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__point32__convert_to_py(void * raw_ros_message);
ROSIDL_GENERATOR_C_IMPORT
bool geometry_msgs__msg__point32__convert_from_py(PyObject * _pymsg, void * _ros_message);
ROSIDL_GENERATOR_C_IMPORT
PyObject * geometry_msgs__msg__point32__convert_to_py(void * raw_ros_message);

ROSIDL_GENERATOR_C_EXPORT
bool clover2_aruco_msgs__msg__marker__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[38];
    {
      char * class_name = NULL;
      char * module_name = NULL;
      {
        PyObject * class_attr = PyObject_GetAttrString(_pymsg, "__class__");
        if (class_attr) {
          PyObject * name_attr = PyObject_GetAttrString(class_attr, "__name__");
          if (name_attr) {
            class_name = (char *)PyUnicode_1BYTE_DATA(name_attr);
            Py_DECREF(name_attr);
          }
          PyObject * module_attr = PyObject_GetAttrString(class_attr, "__module__");
          if (module_attr) {
            module_name = (char *)PyUnicode_1BYTE_DATA(module_attr);
            Py_DECREF(module_attr);
          }
          Py_DECREF(class_attr);
        }
      }
      if (!class_name || !module_name) {
        return false;
      }
      snprintf(full_classname_dest, sizeof(full_classname_dest), "%s.%s", module_name, class_name);
    }
    assert(strncmp("clover2_aruco_msgs.msg._marker.Marker", full_classname_dest, 37) == 0);
  }
  clover2_aruco_msgs__msg__Marker * ros_message = _ros_message;
  {  // id
    PyObject * field = PyObject_GetAttrString(_pymsg, "id");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->id = PyLong_AsUnsignedLong(field);
    Py_DECREF(field);
  }
  {  // length
    PyObject * field = PyObject_GetAttrString(_pymsg, "length");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->length = (float)PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // pose
    PyObject * field = PyObject_GetAttrString(_pymsg, "pose");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__pose__convert_from_py(field, &ros_message->pose)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // c1
    PyObject * field = PyObject_GetAttrString(_pymsg, "c1");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__point32__convert_from_py(field, &ros_message->c1)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // c2
    PyObject * field = PyObject_GetAttrString(_pymsg, "c2");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__point32__convert_from_py(field, &ros_message->c2)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // c3
    PyObject * field = PyObject_GetAttrString(_pymsg, "c3");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__point32__convert_from_py(field, &ros_message->c3)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }
  {  // c4
    PyObject * field = PyObject_GetAttrString(_pymsg, "c4");
    if (!field) {
      return false;
    }
    if (!geometry_msgs__msg__point32__convert_from_py(field, &ros_message->c4)) {
      Py_DECREF(field);
      return false;
    }
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * clover2_aruco_msgs__msg__marker__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of Marker */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("clover2_aruco_msgs.msg._marker");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "Marker");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  clover2_aruco_msgs__msg__Marker * ros_message = (clover2_aruco_msgs__msg__Marker *)raw_ros_message;
  {  // id
    PyObject * field = NULL;
    field = PyLong_FromUnsignedLong(ros_message->id);
    {
      int rc = PyObject_SetAttrString(_pymessage, "id", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // length
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->length);
    {
      int rc = PyObject_SetAttrString(_pymessage, "length", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // pose
    PyObject * field = NULL;
    field = geometry_msgs__msg__pose__convert_to_py(&ros_message->pose);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "pose", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // c1
    PyObject * field = NULL;
    field = geometry_msgs__msg__point32__convert_to_py(&ros_message->c1);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "c1", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // c2
    PyObject * field = NULL;
    field = geometry_msgs__msg__point32__convert_to_py(&ros_message->c2);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "c2", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // c3
    PyObject * field = NULL;
    field = geometry_msgs__msg__point32__convert_to_py(&ros_message->c3);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "c3", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // c4
    PyObject * field = NULL;
    field = geometry_msgs__msg__point32__convert_to_py(&ros_message->c4);
    if (!field) {
      return NULL;
    }
    {
      int rc = PyObject_SetAttrString(_pymessage, "c4", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
