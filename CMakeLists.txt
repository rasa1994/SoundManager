cmake_minimum_required(VERSION 3.29)

#enable_testing()

set(VCPKG_ROOT "VCPKG_ROOT" CACHE STRING "VCPKG_ROOT")

if (NOT DEFINED VCPKG_ROOT)
  message(FATAL_ERROR "MISSING ROOT FOR VCPKG setting to C:/vcpkg")
  set(VCPKG_ROOT "C:/vcpkg")
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project("SoundManager")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ResFolder "Music")

set(RESOURCE_FOLDER "${CMAKE_SOURCE_DIR}/${ResFolder}")

file (GLOB RESOURCES "${RESOURCE_FOLDER}/*")

foreach(resource ${RESOURCES})
  get_filename_component(filename ${resource} NAME)
  configure_file(${resource} ${CMAKE_BINARY_DIR}/${ResFolder}/${filename} COPYONLY)
endforeach()

include_directories(${CMAKE_TOOLCHAIN_FILE})

add_executable(main Source.cpp)
target_link_libraries(main PRIVATE Sound)
target_link_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/SoundController)

add_subdirectory("SoundController")
#add_subdirectory("SoundTests")