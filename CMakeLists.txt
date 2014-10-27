cmake_minimum_required(VERSION 2.4.6)
####include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)
project(lwr_interface)
find_package(catkin REQUIRED COMPONENTS
  #lwr_interface
  robotlib
  #rtk_pkg_tools # not sure why
  simulator_dynamics
  std_tools
  gl_tools
  fri_library_ros  
  kuka_robot_model
)


set(PM_QTMOC_FILES MainWindow ConsoleWidget WorldRendererWidget)
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
SET(QT_USE_QTOPENGL true)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${QT_INCLUDES})
IF(DEFINED PM_QTMOC_FILES)
  SET(MOC_SOURCES "")
  FOREACH(MOC_NAME ${PM_QTMOC_FILES})
    SET(MOC_SOURCES ${MOC_SOURCES} ${PROJECT_SOURCE_DIR}/include/${MOC_NAME}.h)
  ENDFOREACH(MOC_NAME)
  message(STATUS "MOC_SOURCES:  ${MOC_SOURCES}")
  QT4_WRAP_CPP(MOCED_SOURCES
    ${MOC_SOURCES})
ENDIF(DEFINED PM_QTMOC_FILES)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#--------------------------------------------
set(ROS_BUILD_TYPE RelWithDebInfo) # Not sure if that will still work
#--------------------------------------------
####rosbuild_init()
#--------------------------------------------
#set the default path for built executables to the "bin" directory
####set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
####set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#set(lwr_interface PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#--------------------------------------------
#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()
#FILE(GLOB SRC_FILES src/*.cpp)
#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
#--------------------------------------------
####rosbuild_add_boost_directories()
find_package(Boost REQUIRED COMPONENTS thread) ## Other components ?
#--------------------------------------------

#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
#--------------------------------------------
####rosbuild_add_executable(LWRMain src/main.cpp src/LWRCore.cpp include/LWRCore.h)
add_executable(LWRMain src/main.cpp src/LWRCore.cpp include/LWRCore.h)
#rosbuild_add_executable(test33 src/LWRRobotTest.cpp)
#--------------------------------------------
####rosbuild_link_boost(LWRMain thread )S


catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
#  CATKIN_DEPENDS message_runtime
#  CATKIN_DEPENDS mathlib robotlib std_tools
#  DEPENDS system_lib
)

include_directories(include)
include_directories(${catkin_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(LWRMain ${catkin_LIBRARIES} ${Boost_LIBRARIES})


##------------
set_target_properties(LWRMain  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set_target_properties(LWRMain  PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

####rosbuild_find_ros_package(lwr_interface)
####rosbuild_find_ros_package(robotlib)

#--------------------------------------------
####string(REPLACE "/packages/core/robotlib" "" robottoolkit_STACK_PATH ${robotlib_PACKAGE_PATH})
# Not to be used

MESSAGE("lwr_interface_PACKAGE_PATH: " ${PROJECT_SOURCE_DIR})
MESSAGE("robotlib_INCLUDE_DIRS_modified: " ${robotlib_INCLUDE_DIRS}/../../../../bin)

####ADD_CUSTOM_COMMAND(TARGET LWRMain POST_BUILD COMMAND ln -sf ${lwr_interface_PACKAGE_PATH}/bin/LWRMain ${robottoolkit_STACK_PATH}/bin/LWRMain) 

SET(robottoolkit_ROOT_PATH ${robotlib_INCLUDE_DIRS}/../../../../)
## TODO # EDIT this one ...
ADD_CUSTOM_COMMAND(TARGET LWRMain POST_BUILD COMMAND ln -sf ${PROJECT_SOURCE_DIR}/bin/LWRMain ${robottoolkit_ROOT_PATH}/bin/) 
#-----------------------------------------
#create symlinks for data and config..
####rosbuild_find_ros_package(rtk_pkg_tools)
# first create a target that will always be built, that output s nothing but executes a command..
####add_custom_target(dataAndConfig COMMAND ${rtk_pkg_tools_PACKAGE_PATH}/SetupPackage.sh ${PROJECT_NAME})
# make sure this target is built by rosbuild
####add_dependencies(rosbuild_precompile dataAndConfig)


#find path to rtk_pkg_tools
SET(rtk_pkg_tools_ROOT_PATH ${robottoolkit_ROOT_PATH}packages/core/rtk_pkg_tools)
MESSAGE("rtk_pkg_tools_ROOT_PATH: " ${rtk_pkg_tools_ROOT_PATH})

#Execute the configure script (copy data and config folders) # THIS WORKS 10/2014
execute_process(COMMAND ${rtk_pkg_tools_ROOT_PATH}/SetupPackage2.sh ${PROJECT_NAME} )
#----------------------------------------

####FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
#SET(QT_USE_QTOPENGL true)
#INCLUDE(${QT_USE_FILE})
#INCLUDE_DIRECTORIES(${QT_INCLUDES})
#target_link_libraries(LWRMain ${QT_LIBRARIES})
