cmake_minimum_required(VERSION 3.20)
project(clover2_camera_utils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS calib3d core imgproc)

ament_auto_find_build_dependencies()

ament_auto_add_library(${PROJECT_NAME}_core SHARED DIRECTORY "src/calibration")
target_link_libraries(${PROJECT_NAME}_core ${OpenCV_LIBS})

ament_auto_add_library(${PROJECT_NAME}_component SHARED
  src/camera_calibration.cpp
)
target_link_libraries(${PROJECT_NAME}_component
  ${PROJECT_NAME}_core
  ${OpenCV_LIBS}
)

rclcpp_components_register_node(${PROJECT_NAME}_component
  PLUGIN "clover2_camera_utils::camera_calibration"
  EXECUTABLE camera_calibration
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  file(GLOB test_source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test/*.cpp")
  if(test_source)
    ament_auto_add_gtest(test_${PROJECT_NAME}
      ${test_source}
    )
    target_link_libraries(test_${PROJECT_NAME}
      ${PROJECT_NAME}_core
      ${PROJECT_NAME}_component
    )
  endif()
endif()

ament_auto_package(
  USE_SCOPED_HEADER_INSTALL_DIR
)
