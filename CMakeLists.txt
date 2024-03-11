
cmake_minimum_required(VERSION 3.27)
project(weather_gov VERSION "0.1.0" LANGUAGES "CXX")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_directories(/usr/local/lib64/mariadb)

set(CMAKE_CXX_FLAGS "-g -O2 -Wall")
set (CMAKE_SHARED_LINKER_FLAGS "-I/usr/local/lib64/mariadb -lmariadbcpp")


set(SOURCES
	src/weather_gov.cpp
	src/station.cpp
	src/config.cpp
	src/db.cpp
	include/logger.h
)

#set(TEST_SOURCES
#    src/tests.cpp
#    src/utf8.cpp
#)

set(INCLUDES
	include
)

add_executable(weather_gov ${SOURCES})
target_link_libraries(weather_gov PUBLIC mariadbcpp)

include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  #GIT_TAG <tag_name> # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
  GIT_TAG master # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()
target_link_libraries(weather_gov PUBLIC yaml-cpp::yaml-cpp) # The library or executable that require yaml-cpp library


FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG master) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)
target_link_libraries(weather_gov PUBLIC cpr::cpr) # The library or executable that require cpr library

add_custom_target(copy-test-config ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/weather_gov.yml)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/weather_gov.yml
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/weather_gov.yml
                                                    ${CMAKE_CURRENT_BINARY_DIR}/weather_gov.yml)
#                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/weather_gov.yml)

#add_executable(tests ${TEST_SOURCES})
#target_include_directories(tests PRIVATE ${INCLUDES})
#target_include_directories(utf8 PRIVATE ${INCLUDES})
target_include_directories(weather_gov PRIVATE ${INCLUDES})


