add_library(Sound STATIC Sound.ixx)
add_library(SoundController STATIC SoundController.ixx)

find_package(OpenAL CONFIG REQUIRED)
target_link_libraries(SoundController PUBLIC OpenAL::OpenAL)
message(STATUS "OpenAL include dir: ${OpenAL_DIR}")
target_include_directories(SoundController PUBLIC ${OpenAL_DIR})

target_link_libraries(Sound PUBLIC SoundController)
target_include_directories(Sound PUBLIC ${CMAKE_SOURCE_DIR}/SoundController)