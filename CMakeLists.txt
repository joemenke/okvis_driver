# ubuntu 12.04 LTS cmake version 2.8.7
# ubuntu 14.04 LTS cmake version 2.8.12.2
# ubuntu 16.04 LTS cmake version 3.5.1
cmake_minimum_required(VERSION 2.8.3)

project(okvisdriver)
set(CMAKE_CXX_STANDARD 11)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# View the makefile commands during build
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

set(CMAKE_BUILD_TYPE "release")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(OpenGL REQUIRED)

find_package(OpenCV REQUIRED)
IF(OpenCV_FOUND)
   MESSAGE(STATUS "Found OpenCV: ${OpenCV_INCLUDE_DIRS}")
ENDIF(OpenCV_FOUND)

find_package(Eigen REQUIRED)
IF(Eigen_FOUND)
   MESSAGE(STATUS "Found Eigen: ${EIGEN_INCLUDE_DIRS}")
ENDIF(Eigen_FOUND)
#SET( EIGEN_INCLUDE_DIRS "$ENV{EIGEN_INCLUDE_DIR}" )

find_package(Ceres REQUIRED)
IF(Ceres_FOUND)
   MESSAGE(STATUS "Found Ceres: ${CERES_INCLUDE_DIRS}")
ENDIF(Ceres_FOUND)

# require okvis
find_package( okvis 1.1 REQUIRED)
include_directories(${OKVIS_INCLUDE_DIRS})
message(STATUS "Okvis at ${OKVIS_INCLUDE_DIRS}.")

# require brisk
find_package( brisk 2 REQUIRED)
include_directories(${BRISK_INCLUDE_DIRS})
message(STATUS "Brisk at ${BRISK_INCLUDE_DIRS}.")

# require OpenGV
find_package(opengv REQUIRED)
IF(OPENGV_FOUND)
   MESSAGE(STATUS "Found OpenGV: ${OPENGV_LIBS}")
ENDIF(OPENGV_FOUND)

find_package(DBoW2 REQUIRED)
IF(DBoW2_FOUND)
   MESSAGE(STATUS "Found DBoW2: ${DBoW2_INCLUDE_DIRS}")
ENDIF(DBoW2_FOUND)

find_package(DLib REQUIRED)
IF(DLib_FOUND)
   MESSAGE(STATUS "Found DLib: ${DLib_LIBRARIES}")
ENDIF(DLib_FOUND)

if(WIN32)
   set(Boost_USE_STATIC_LIBS ON)
   set(Boost_USE_STATIC ON)
   set(Boost_USE_MULTITHREAD )
endif(WIN32)

find_package(Boost REQUIRED COMPONENTS filesystem date_time)
IF(Boost_FOUND)
	MESSAGE(STATUS "Found Boost: ${Boost_INCLUDE_DIR}")
ENDIF(Boost_FOUND)

set(
  DEPENDENCIES 
  ${OPENGL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${EIGEN_LIBRARIES}
  ${CERES_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OKVIS_LIBRARIES}
  ${DLib_LIBRARIES}
  ${BRISK_LIBRARIES})

if(WIN32)
    add_subdirectory(src/third_party/glfw)
    list(APPEND DEPENDENCIES glfw3)
else()
    # Find glfw header
    find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h
        PATHS /usr/X11R6/include
              /usr/include/X11
              /opt/graphics/OpenGL/include
              /opt/graphics/OpenGL/contrib/libglfw
              /usr/local/include
              /usr/include/GL
              /usr/include
    )
    # Find glfw library
    find_library(GLFW_LIBRARIES NAMES glfw glfw3
            PATHS /usr/lib64
                  /usr/lib
                  /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
                  /usr/local/lib64
                  /usr/local/lib
                  /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
                  /usr/X11R6/lib
    )
    list(APPEND DEPENDENCIES m ${GLFW_LIBRARIES} ${LIBUSB1_LIBRARIES})
    include_directories(${GLFW_INCLUDE_DIR})
endif()

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${DBoW2_INCLUDE_DIRS}
  ${DLib_INCLUDE_DIRS}
  ${OKVIS_INCLUDE_DIRS}
  ${BRISK_INCLUDE_DIRS}
)

set(
  SOURCES
  src/util/time_profiler.cpp
  src/util/glfwManager.cpp

)
add_executable(okvis_driver src/okvis_driver.cpp ${SOURCES})
target_link_libraries(okvis_driver ${DEPENDENCIES})

install(
    TARGETS
    okvis_driver

    RUNTIME DESTINATION
    ${CMAKE_BINARY_DIR}
)


