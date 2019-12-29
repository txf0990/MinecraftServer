# A plain program without boost
add_executable(hello_world hello_world.cpp)

# Boost tutorial
add_executable(timer timer.cpp)
target_link_libraries (timer ${COMMON_LIBS})

add_executable(daytime_server daytime_server.cpp)
target_link_libraries (daytime_server ${COMMON_LIBS})

add_executable(daytime_client daytime_client.cpp)
target_link_libraries (daytime_client ${COMMON_LIBS})

add_executable(test_client test_client.cpp)
target_link_libraries (test_client ${COMMON_LIBS})

add_executable(test_server test_server.cpp)
target_link_libraries (test_server ${COMMON_LIBS})
