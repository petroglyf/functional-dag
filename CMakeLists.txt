cmake_minimum_required(VERSION 3.6)
project(functional-dag LANGUAGES CXX )

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories("include/")
add_compile_options("-stdlib=libc++")

option (DEBUG "Build debug version" ON)
option (RELEASE "Build release version" OFF)

if (DEBUG)
  add_definitions(-g -pg)
endif ()

if (RELEASE)
  add_definitions(-Ofast)
endif ()


##########################################
####### Find the needed packages #########
##########################################
find_package(PkgConfig REQUIRED)
find_package(Catch2 3 REQUIRED)
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


########################################
####### Building specification #########
########################################
add_library(filterdag STATIC src/filter_sys/dag_manager.cpp src/filter_sys/libutils.cpp)
target_include_directories(filterdag PRIVATE "include/" ${JSONCPP_INCLUDE_DIRS})


########################################
####### Build the testing files ########
########################################
if (DEFINED Catch2_DIR)
  add_executable(tests test/filter_sys/dag_tests.cpp test/filter_sys/lib_tests.cpp)
  target_include_directories(tests PRIVATE "include/" ${CATCH_INCLUDE_DIR})
  target_link_directories(tests PRIVATE ${CATCH_LIBRARY_DIRS} ${JSONCPP_LIBRARY_DIRS})
  target_link_libraries(tests filterdag Catch2::Catch2WithMain ${JSONCPP_LIBRARIES})

  include(CTest)
  include(Catch)
  catch_discover_tests(tests)
endif()


########################################
####### Install specification ##########
########################################
target_sources(filterdag PUBLIC FILE_SET HEADERS
  BASE_DIRS include/
  FILES 
    include/filter_sys/filter_sys.hpp 
    include/filter_sys/dag_interface.hpp
    include/filter_sys/dag_fanout_impl.hpp
    include/filter_sys/dag_node_impl.hpp
    include/filter_sys/dag_impl.hpp
    include/filter_sys/lib_utils.h
    include/filter_sys/dlpack.h
    include/filter_sys/dag_utils.hpp)

install(TARGETS filterdag 
    EXPORT filterdagTargets
    FILE_SET HEADERS
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT filterdagTargets
    FILE FilterDagConfig.cmake
    NAMESPACE FilterDag::
    DESTINATION lib/cmake/filterdag)