cmake_minimum_required(VERSION 3.8)

project(clover2_cam_feature)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS aruco)

ament_auto_find_build_dependencies()

ament_auto_add_library(cam_feature_base SHARED src/base_plugin.cpp)
target_link_libraries(cam_feature_base ${OpenCV_LIBS})

ament_auto_add_library(cam_feature_plugins SHARED
  src/detail/maker_base.cpp
  src/plugins/aruco.cpp
)
target_link_libraries(cam_feature_plugins
  cam_feature_base
  ${OpenCV_LIBS}
)

pluginlib_export_plugin_description_file(${PROJECT_NAME} plugins.xml)

ament_auto_add_library(clover2_cam_feature_component SHARED
  src/cam_feature.cpp
  src/diagnostics/markers_task.cpp
)
target_link_libraries(clover2_cam_feature_component
  cam_feature_plugins
  cam_feature_base
  ${OpenCV_LIBS}
)
rclcpp_components_register_node(clover2_cam_feature_component
  PLUGIN "clover2::cam_feature::cam_feature"
  EXECUTABLE cam_feature
)

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
