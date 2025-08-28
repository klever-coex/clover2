#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "clover2_aruco::clover2_aruco_detector" for configuration ""
set_property(TARGET clover2_aruco::clover2_aruco_detector APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(clover2_aruco::clover2_aruco_detector PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libclover2_aruco_detector.so"
  IMPORTED_SONAME_NOCONFIG "libclover2_aruco_detector.so"
  )

list(APPEND _cmake_import_check_targets clover2_aruco::clover2_aruco_detector )
list(APPEND _cmake_import_check_files_for_clover2_aruco::clover2_aruco_detector "${_IMPORT_PREFIX}/lib/libclover2_aruco_detector.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
