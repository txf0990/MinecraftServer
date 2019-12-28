# A plain program without boost
add_executable(hello_world hello_world.cpp)

# Boost tutorial
add_executable(timer timer.cpp)
target_link_libraries (timer ${COMMON_LIBS})

