cmake_minimum_required(VERSION 2.4.6)
####include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)
project(lwr_interface)
find_package(catkin REQUIRED COMPONENTS
  robotlib
  rtk_pkg_tools # Someone: not sure why, Klas: because the linking should be correct, see end of this file 02/2015
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


find_package(Boost REQUIRED COMPONENTS thread) ## Other components ?



catkin_package(
  INCLUDE_DIRS include
#  LIBRARIES ${PROJECT_NAME}
#  CATKIN_DEPENDS message_runtime
#  CATKIN_DEPENDS mathlib robotlib std_tools
#  DEPENDS system_lib
)

include_directories(include)
include_directories(${catkin_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_executable(lwr_main src/main.cpp src/LWRCore.cpp include/LWRCore.h)
target_link_libraries(lwr_main ${catkin_LIBRARIES} ${Boost_LIBRARIES})

# link the executable to the traditional location
set(ROBOTTOOLKIT_PATH ${rtk_pkg_tools_SOURCE_DIR}/..)
# this way it does not matter if the original build is in source or not.
get_property(exec_output_dir TARGET lwr_main PROPERTY RUNTIME_OUTPUT_DIRECTORY)
add_custom_command(TARGET lwr_main POST_BUILD COMMAND ln -sf ${exec_output_dir}/lwr_main ${ROBOTTOOLKIT_PATH}/bin/lwr_main )
