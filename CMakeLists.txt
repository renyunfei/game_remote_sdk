cmake_minimum_required(VERSION 3.4.3) 
project(devsdk)
link_libraries(event event_pthreads pthread m)
file(GLOB SRCS *.c *.h)
add_executable( ${PROJECT_NAME} ${SRCS} )
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
    )
endif ()
