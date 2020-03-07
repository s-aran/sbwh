cmake_minimum_required(VERSION 3.10)

project(sbwh CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# sbwh
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sbwh)
add_executable(sbwh ${SRC_DIR}/sbwh.cpp)

# pthread
find_package(Threads REQUIRED)
target_link_libraries(sbwh ${CMAKE_THREAD_LIBS_INIT})

# boost
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)
find_package(Boost COMPONENTS regex filesystem)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(sbwh ${Boost_LIBRARIES})
else()
  message(FATAL_ERROR "Boost not found.")
endif()

# mustache
find_package(mstch REQUIRED)
if(mstch_FOUND)
  include_directories(${mstch_DIR}/../../../include/)
  target_link_libraries(sbwh ${mstch_DIR}/../../libmstch.a)
endif()

