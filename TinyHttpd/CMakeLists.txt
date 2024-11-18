cmake_minimum_required(VERSION 3.5.0)
project(TinyHttpd)

set (TAGET TinyHttpdDemo)

add_definitions(-g)
add_compile_options(-fPIC)

# open debug
# set(CMAKE_C_FLAGS "-Wall -O0 -ggdb")

set(PROJECT_TOP_DIR ${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_INSTALL_INCLUDEDIR})

set(SRCS
	${PROJECT_TOP_DIR}/main.cpp
	${PROJECT_TOP_DIR}/TcpServer.cpp
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_LIBDIR})

add_executable(${TAGET}
                        ${SRCS}
                        )
target_link_libraries(${TAGET} 
	-pthread 
	)
