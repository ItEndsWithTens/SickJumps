### Basic info ###

cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

if(WIN32)
  project(SickJumps CXX)
else()
  project(sickjumps CXX)
endif()

set(SICKJUMPS_MAJOR 0)
set(SICKJUMPS_MINOR 1)
set(SICKJUMPS_PATCH 0)
set(SICKJUMPS_VERSION ${SICKJUMPS_MAJOR}.${SICKJUMPS_MINOR}.${SICKJUMPS_PATCH})
set(SICKJUMPS_AUTHOR "Robert Martens")
set(SICKJUMPS_AUTHOR_EMAIL "<robert.martens@gmail.com>")

set(HOSTS "")
if(WIN32)
  if(${CMAKE_GENERATOR} MATCHES "Win64")
    list(APPEND HOSTS "AvisynthPlus x64")
endif()

list(SORT HOSTS)
list(GET HOSTS 0 HOST_DEFAULT)
set(SICKJUMPS_HOST ${HOST_DEFAULT} CACHE STRING "Host application to target.")
set_property(CACHE SICKJUMPS_HOST PROPERTY STRINGS ${HOSTS})

set(SICKJUMPS_DESCRIPTION "SickJumps plugin for ${SICKJUMPS_HOST}")


### Source files ###

set(SICKJUMPS_HOST_INCLUDE_DIR "" CACHE PATH "Directory that contains avisynth.h.")
set(SICKJUMPS_HOST_HDR ${SICKJUMPS_HOST_INCLUDE_DIR}"/avisynth.h")
include_directories(${SICKJUMPS_HOST_INCLUDE_DIR})

set(SRCS
  ${SICKJUMPS_HOST_HDR}
  src/interface.cpp
  src/SickJumps.h
  src/SickJumps.cpp
  src/SickJumpsCore.h
  src/SickJumpsCore.cpp
)

if(WIN32)
  configure_file(
      src/SickJumps.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/src/SickJumps.rc
  )
  list(APPEND SRCS src/SickJumps.rc)
  if(MSVC)
    # CMake handles the Source and Header Files filters automatically, but not
    # Resource Files, the way VS does. This is just cosmetic, really.
    source_group("Resource Files" FILES src/SickJumps.rc)
  endif()
  
endif()

string(TOUPPER ${SICKJUMPS_HOST} SICKJUMPS_HOST_DEFINE)
string(REPLACE "." "" SICKJUMPS_HOST_DEFINE ${SICKJUMPS_HOST_DEFINE})
string(REPLACE " " "_" SICKJUMPS_HOST_DEFINE ${SICKJUMPS_HOST_DEFINE})

# When defined as SHARED, the plugin builds correctly in Windows and Linux, but
# it is technically a MODULE, and that distinction is important in macOS.
add_library(${PROJECT_NAME} SHARED ${SRCS})
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  COMPILE_DEFINITIONS "SICKJUMPS_HOST_${SICKJUMPS_HOST_DEFINE}"
)



### CMake options ###
if(MSVC)
  
  # Dynamic linking is safer, relying on hopefully-up-to-date versions of the
  # runtime library, but static linking can be more convenient, hence an option.
  option(
    SICKJUMPS_STATIC_CRT "Link statically with the C Run-Time libraries." FALSE
  )
  
  if(SICKJUMPS_STATIC_CRT)
    
    foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
    
  else()
  
    foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
    endforeach()
    
  endif()
  
endif()

if(WIN32)
  
    get_filename_component(
      HOST_PLUGIN_DIR
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\AviSynth;plugindir2_5] ABSOLUTE
    )
    
  endif()
  
endif()

# If Avisynth isn't installed, the registry entry won't be there, and the
# HOST_PLUGIN_DIR variable will be set to "/registry", which is no good.
if(NOT DEFINED HOST_PLUGIN_DIR OR
   NOT EXISTS "${HOST_PLUGIN_DIR}" OR
   NOT IS_DIRECTORY "${HOST_PLUGIN_DIR}")
  set(HOST_PLUGIN_DIR "")
endif()
  
# This is overkill, perhaps, but I want my tools to be convenient; in this case,
# that means SICKJUMPS_PLUGINS_DIR should be automatically defaulted not just
# the first time, but on every change of the host option, which means I need to
# keep track of the previous value for later comparison.
if(NOT DEFINED SICKJUMPS_HOST_PREV)
  set(
    SICKJUMPS_HOST_PREV "Dummy string" CACHE INTERNAL
    "Previously defined host application."
  )
endif()

# Checking the validity of manually entered plugin directories makes this whole
# process a lot more complicated than I thought it would, so for now I simply
# trust that users know what they're doing when choosing their own path...
if(NOT DEFINED SICKJUMPS_PLUGINS_DIR OR
   NOT "${SICKJUMPS_HOST}" STREQUAL "${SICKJUMPS_HOST_PREV}")
  set(
   SICKJUMPS_PLUGINS_DIR ${HOST_PLUGIN_DIR} CACHE PATH
   "Path to your plugin directory (for more convenient debugging). Leave blank to disable ${PROJECT_NAME}-copy target." FORCE
  )
  set(
   SICKJUMPS_HOST_PREV ${SICKJUMPS_HOST} CACHE INTERNAL
   "Previously defined host application."
  )
endif()

# ...though I still try to protect people (like me) who might use backslashes.
if(NOT "${SICKJUMPS_PLUGINS_DIR}" STREQUAL "")
  file(TO_CMAKE_PATH ${SICKJUMPS_PLUGINS_DIR} SICKJUMPS_PLUGINS_DIR)
endif()



### Testing ###

# This custom target basically duplicates the result of the CMake-provided
# INSTALL target, but saves you some annoying little extra effort. By setting
# the ALL dependency, the standard ALL_BUILD target now depends on this one,
# so that every time you build the project, the resulting library will be copied
# to the plugins directory, even if the plugin is up to date, and even if you're
# only switching between configurations, making debugging a one key affair.
if(NOT "${SICKJUMPS_PLUGINS_DIR}" STREQUAL "")
  add_custom_target(
    ${PROJECT_NAME}-copy ALL
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${PROJECT_NAME}> ${SICKJUMPS_PLUGINS_DIR}
  )
endif()

option(SICKJUMPS_TESTS "Enable testing with Catch framework." TRUE)
if(SICKJUMPS_TESTS)
  set(SRCS_TEST
    test/include/catch/catch.hpp
    
    ${SICKJUMPS_HOST_HDR}
    test/src/main.cpp
    test/src/frames.cpp
    test/src/samples.cpp
  )
  
  include_directories(
    test/include
    src
  )
  
  set(
    SICKJUMPS_TEST_DESCRIPTION
    "Test executable for the ${SICKJUMPS_DESCRIPTION}"
  )
  
  if(WIN32)
    configure_file(
      test/src/SickJumps-test.rc.in
      ${CMAKE_CURRENT_SOURCE_DIR}/test/src/SickJumps-test.rc
    )
    list(APPEND SRCS_TEST test/src/SickJumps-test.rc)
    if(MSVC)
      source_group("Resource Files" FILES test/src/SickJumps-test.rc)
    endif()
  endif()
  
  add_executable(${PROJECT_NAME}-test ${SRCS_TEST})
  
  target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
  
  # This ensures the tests are run after every build; I need to target my custom
  # -copy target instead of the project itself to make sure the latest version
  # of the plugin is the one being tested.
  add_custom_command(TARGET ${PROJECT_NAME}-copy POST_BUILD
    COMMAND ${PROJECT_NAME}-test
  )

  set_target_properties(
    ${PROJECT_NAME}-test
    PROPERTIES
    COMPILE_DEFINITIONS "SICKJUMPS_HOST_${SICKJUMPS_HOST_DEFINE}"
  )
endif()



### Packaging options ###

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

set(CPACK_SOURCE_IGNORE_FILES
  # Directories
  "\\\\.git"
  "build"
  "scratch"
  "dist"
  # Project files
  "gitignore$"
  "\\\\.db$"
  "\\\\.kdev"
  "\\\\.ffindex$"
  # Source
  "\\\\.rc$"
  "~$"
  # Packages
  "zip$"
)

set(CMAKE_INSTALL_PREFIX ${SICKJUMPS_PLUGINS_DIR} CACHE INTERNAL "")

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

install(
  FILES
  README.txt
  DESTINATION .
  RENAME ${PROJECT_NAME}.txt
)

# Build the source package by changing to your build directory and
# running: cpack --config CPackSourceConfig.cmake
set(CPACK_SOURCE_GENERATOR "ZIP")
set(SICKJUMPS_HOST_ABBR "AVS")

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/dist)

set(CPACK_PACKAGE_VERSION_MAJOR ${SICKJUMPS_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SICKJUMPS_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SICKJUMPS_PATCH})
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(SICKJUMPS_ARCH x64)
else()
  set(SICKJUMPS_ARCH x86)
endif()

set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${SICKJUMPS_VERSION}-${SICKJUMPS_HOST_ABBR}${SICKJUMPS_HOST_VERSION}-${CMAKE_SYSTEM_NAME}-${SICKJUMPS_ARCH})

include(CPack)
