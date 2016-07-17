cmake_minimum_required(VERSION 2.8)

project(tinyflock)

find_package(Threads REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRECTORIES})
link_directories(${GLUT_LIBRARY_DIRECTORIES})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
	message(FATAL_ERROR "GLUT not found")
endif(NOT GLUT_FOUND)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_directories(${OPENGL_LIBRARY_DIRS})
add_definitions(${OPENGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL not found")
endif(NOT OPENGL_FOUND)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "-std=gnu11 -O3 ${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} ${OPENGL_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
if(NOT OPENMP_FOUND)
	message(FATAL_ERROR "OpenMP not found")
endif(NOT OPENMP_FOUND)

FILE(GLOB SOURCE_FILES src/*.c)

add_executable(
	tinyflock
	${SOURCE_FILES})

add_subdirectory(glfw)

if(IS_DIRECTORY fann)
    add_subdirectory(fann)
    add_definitions(-DENABLE_ANN)
else(IS_DIRECTORY fann)
    message(WARNING "FANN not found, disabling ANN features")
endif(IS_DIRECTORY fann)


include_directories(include)
include_directories(glfw/include)
include_directories(src)

target_link_libraries(tinyflock glfw fann
	${GLFW_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENGL_LIBRARIES} ${FANN_LIBRARIES})

