cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()
#FILE(GLOB SRC_FILES src/*.cpp)
#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
rosbuild_add_boost_directories()

#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
rosbuild_add_executable(LWRMain src/main.cpp src/LWRCore.cpp include/LWRCore.h)
#rosbuild_add_executable(test33 src/LWRRobotTest.cpp)

rosbuild_link_boost(LWRMain thread )


rosbuild_find_ros_package(LWRInterface)

rosbuild_find_ros_package(RobotLib)

string(REPLACE "/packages/core/RobotLib" "" RobotToolKit_STACK_PATH ${RobotLib_PACKAGE_PATH})


ADD_CUSTOM_COMMAND(TARGET LWRMain POST_BUILD COMMAND ln -sf ${LWRInterface_PACKAGE_PATH}/bin/LWRMain ${RobotToolKit_STACK_PATH}/bin/LWRMain) 

#create symlinks for data and config..
rosbuild_find_ros_package(RTK_pkg_tools)
# first create a target that will always be built, that output s nothing but executes a command..
add_custom_target(dataAndConfig COMMAND ${RTK_pkg_tools_PACKAGE_PATH}/SetupPackage.sh ${PROJECT_NAME})
# make sure this target is built by rosbuild
add_dependencies(rosbuild_precompile dataAndConfig)


FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
SET(QT_USE_QTOPENGL true)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${QT_INCLUDES})
target_link_libraries(LWRMain ${QT_LIBRARIES})
