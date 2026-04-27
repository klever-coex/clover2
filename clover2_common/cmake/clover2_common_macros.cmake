include(CMakeParseArguments)

macro(clover2_target_include_dirs target)
  target_include_directories(${target} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
endmacro()

macro(clover2_add_library)
  set(options)
  set(oneValueArgs TARGET ALIAS)
  set(multiValueArgs SOURCES DEFINES LINK_LIBS DEPENDENCIES)
  cmake_parse_arguments(CLOVER2 "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(NOT CLOVER2_TARGET)
    message(FATAL_ERROR "clover2_add_library: TARGET is required")
  endif()
  if(NOT CLOVER2_SOURCES)
    message(FATAL_ERROR "clover2_add_library: SOURCES is required")
  endif()

  add_library(${CLOVER2_TARGET} ${CLOVER2_SOURCES})
  if(CLOVER2_ALIAS)
    add_library(${CLOVER2_ALIAS} ALIAS ${CLOVER2_TARGET})
  endif()

  target_compile_features(${CLOVER2_TARGET} PUBLIC c_std_99 cxx_std_17)
  clover2_target_include_dirs(${CLOVER2_TARGET})

  if(CLOVER2_DEFINES)
    target_compile_definitions(${CLOVER2_TARGET} PRIVATE ${CLOVER2_DEFINES})
  endif()

  if(CLOVER2_LINK_LIBS)
    target_link_libraries(${CLOVER2_TARGET} ${CLOVER2_LINK_LIBS})
  endif()

  if(CLOVER2_DEPENDENCIES)
    ament_target_dependencies(${CLOVER2_TARGET} ${CLOVER2_DEPENDENCIES})
  endif()
endmacro()

macro(clover2_add_component_library)
  set(options)
  set(oneValueArgs TARGET COMPONENT_CLASS)
  set(multiValueArgs SOURCES DEFINES LINK_LIBS DEPENDENCIES)
  cmake_parse_arguments(CLOVER2 "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  clover2_add_library(
    TARGET ${CLOVER2_TARGET}
    SOURCES ${CLOVER2_SOURCES}
    DEFINES ${CLOVER2_DEFINES}
    LINK_LIBS ${CLOVER2_LINK_LIBS}
    DEPENDENCIES ${CLOVER2_DEPENDENCIES})

  if(CLOVER2_COMPONENT_CLASS)
    rclcpp_components_register_nodes(${CLOVER2_TARGET}
      "${CLOVER2_COMPONENT_CLASS}")
  endif()
endmacro()

macro(clover2_add_executable)
  set(options)
  set(oneValueArgs TARGET)
  set(multiValueArgs SOURCES LINK_LIBS DEPENDENCIES)
  cmake_parse_arguments(CLOVER2 "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

  if(NOT CLOVER2_TARGET)
    message(FATAL_ERROR "clover2_add_executable: TARGET is required")
  endif()
  if(NOT CLOVER2_SOURCES)
    message(FATAL_ERROR "clover2_add_executable: SOURCES is required")
  endif()

  add_executable(${CLOVER2_TARGET} ${CLOVER2_SOURCES})
  clover2_target_include_dirs(${CLOVER2_TARGET})

  if(CLOVER2_LINK_LIBS)
    target_link_libraries(${CLOVER2_TARGET} ${CLOVER2_LINK_LIBS})
  endif()

  if(CLOVER2_DEPENDENCIES)
    ament_target_dependencies(${CLOVER2_TARGET} ${CLOVER2_DEPENDENCIES})
  endif()
endmacro()

