cmake_minimum_required(VERSION 3.2)
project(sdata VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "-std=gnu++2a -pthread -pg -g")

add_library(sdata SHARED sdata.cpp) 

configure_file("sdata.pc.in" "sdata.pc" @ONLY)

# Generate the config file
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/sdataConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sdataConfig.cmake
  INSTALL_DESTINATION lib/cmake/sdata
)

# Generate the version file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/sdataConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(TARGETS sdata EXPORT sdataTargets LIBRARY DESTINATION lib) 
install(FILES sdata.h DESTINATION include) 
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdata.pc" DESTINATION lib/pkgconfig) 

# Install the config files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdataConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/sdataConfigVersion.cmake
        DESTINATION lib/cmake/sdata)

# Install the export set for use with the install-tree
install(EXPORT sdataTargets
        FILE sdataTargets.cmake
        DESTINATION lib/cmake/sdata)