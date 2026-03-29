cmake_minimum_required(VERSION 3.8)
project(clover2_cam_feature)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(clover2_map_server REQUIRED)
find_package(clover2_common REQUIRED)
find_package(clover2_localization_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(diagnostic_updater REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(image_geometry REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS aruco)

set(dependencies
    clover2_map_server
    clover2_common
    clover2_localization_msgs
    cv_bridge
    diagnostic_updater
    geometry_msgs
    image_geometry
    pluginlib
    rclcpp
    rclcpp_components
    rclcpp_lifecycle
    sensor_msgs
    tf2
)

add_library(cam_feature_plugins SHARED
    src/detail/maker_base.cpp
    src/plugins/aruco.cpp
)

target_include_directories(cam_feature_plugins PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_target_dependencies(cam_feature_plugins ${dependencies})

target_link_libraries(cam_feature_plugins ${OpenCV_LIBRARIES})

pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

clover2_add_component_library(
    TARGET clover2_cam_feature_component
    SOURCES src/cam_feature.cpp
    LINK_LIBS cam_feature_plugins ${OpenCV_LIBRARIES}
    DEPENDENCIES ${dependencies}
    COMPONENT_CLASS "clover2_cam_feature::cam_feature"
)

clover2_add_executable(
  TARGET cam_feature
  SOURCES src/main.cpp
  LINK_LIBS clover2_cam_feature_component opencv_aruco
  DEPENDENCIES ${dependencies}
)

install(
    TARGETS cam_feature_plugins clover2_cam_feature_component
    EXPORT export_${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(
  TARGETS cam_feature
  DESTINATION lib/${PROJECT_NAME}
)

install(
    DIRECTORY include/
    DESTINATION include)

ament_export_include_directories(include)
ament_export_libraries(cam_feature_plugins clover2_cam_feature_component)
ament_export_targets(export_${PROJECT_NAME})
ament_export_dependencies(${dependencies})

if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    set(ament_cmake_copyright_FOUND TRUE)
    set(ament_cmake_cpplint_FOUND TRUE)
    ament_lint_auto_find_test_dependencies()
endif()

ament_package()
