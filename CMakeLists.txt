CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(ron_core LANGUAGES C CXX VERSION 1.0.0)

INCLUDE(ExternalProject)

# Require a C++ compiler that supports at least C++17 standard
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_STANDARD_REQUIRES ON)

# Set the Compilation flags.
SET(CMAKE_C_FLAGS "-std=gnu14 -fms-extensions")
SET(CMAKE_CXX_FLAGS "-std=gnu++14 -fms-extensions -pthread")

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Create compile_commands.json file so that it may be used by various
# editors/plugins/IDEs that support it.
SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Prefer -pthread compiler and linker flag when using libpthread. This must
# be set before call to FIND_PACKAGE(Threads).
#SET(THREADS_HAVE_PTHREAD_ARG 1)
#SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#FIND_PACKAGE(Threads REQUIRED)


SET(LIBPREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
SET(LIBSUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LD_VERSION_SCRIPT_FLAG}")

INCLUDE_DIRECTORIES(
  src
  ${source_dir}/libraries
  )

ADD_SUBDIRECTORY(src)
