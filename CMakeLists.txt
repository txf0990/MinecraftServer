project (TryBoost)

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 17)

set(Boost_ADDITIONAL_VERSIONS "1.72" "1.72.0")
set(Boost_DEBUG on)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.67.0 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
    message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
    message(status "** Boost Libraries: ${Boost_LIBRARIES}")
else()
    message(STATUS "Unable to find boost!")
endif()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(COMMON_LIBS ${Boost_LIBRARIES})

message(status "** env = ${CMAKE_CXX_COMPILER_ID}")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    message(status "** link pthread")
    set (COMMON_LIBS ${COMMON_LIBS} pthread)
endif()

#add_executable(main main.cpp)
#target_link_libraries(main ${COMMON_LIBS})
add_subdirectory(src)
