cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME Linux)

project(GALasmII VERSION 2.1.2)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

option(RELEASE_BUILD "Release build " OFF)
option(BUILD_UNITTEST "Build unittests " OFF)
option(BUILD_SHARED  "Build as shared library" OFF)
option(ENABLE_SANITIZER "Enable address sanitizer" OFF)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if (BUILD_UNITTEST)
  enable_testing()
endif()

if (BUILD_SHARED)
  set(BUILD_TYPE "SHARED")
else()
  set(BUILD_TYPE "STATIC")
endif()

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
  ${CMAKE_SOURCE_DIR}/ProjectVersion.h.in
  ${CMAKE_BINARY_DIR}/ProjectVersion.h
)

include_directories(${CMAKE_BINARY_DIR})

# Common compiler options for debug and release builds
#
set(flag " ")
# set(flag "${flag} -Werror")
set(flag "${flag} -Wpointer-arith")
set(flag "${flag} -Wno-shadow ")
set(flag "${flag} -Wcast-qual")
set(flag "${flag} -Wsign-compare")
set(flag "${flag} -Wunused-variable")
set(flag "${flag} -Wunused-value")
set(flag "${flag} -Wunused-label")
set(flag "${flag} -Wparentheses")
set(flag "${flag} -Wreturn-type")
set(flag "${flag} -Wno-unused-function")
set(flag "${flag} -Wunused-const-variable")
set(flag "${flag} -Wcast-align")

# C only flags
set(flag_c "${flag_c} -Wmissing-prototypes")
set(flag_c "${flag_c} -Wno-shift-count-overflow")

# CPP only flags
set(flag_cpp "${flag_cpp} -Wreorder")

if(RELEASE_BUILD)
  message(WARNING "  === Release build === ")
  set(flag "${flag} -O3")
  set(flag "${flag} -DNDEBUG")
else()
  message(WARNING "  === Debug build === ")
  set(CMAKE_BUILD_TYPE Debug)

  set(flag "${flag} -Wall")
  set(flag "${flag} -g ")
  set(flag "${flag} -O0")

  if(ENABLE_SANITIZER)
    message(WARNING "  === sanitizer is enabled === ")
    # http://clang.llvm.org/docs/AddressSanitizer.html
    # set(flag "${flag} -fsanitize=leak -fno-omit-frame-pointer")
    # set(flag "${flag} -fsanitize=thread -fno-omit-frame-pointer")
    set(flag "${flag} -fsanitize=address -fno-omit-frame-pointer")
  endif()
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${flag} ${flag_c} " CACHE STRING "compile flags" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag} ${flag_cpp} " CACHE STRING "compile flags" FORCE)

set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(INSTALL_TEST_DIR "${CMAKE_INSTALL_PREFIX}/test")

add_subdirectory(galasm)
add_subdirectory(galasm_main)

# This section generates a install package
include (InstallRequiredSystemLibraries)

set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CPACK_RPM_SPEC_INSTALL_POST "/bin/true")
endif()

message(WARNING "  === Building Debian package === ")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Egil Rasmussen")
# set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_CURRENT_SOURCE_DIR}/postinst)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}.${GIT_COMMIT_HASH}")

# Build a CPack driven installer package
include (CPack)

