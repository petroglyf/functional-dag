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
add_library(functional_dag SHARED src/functional_dag/dag_manager.cpp 
                                  src/functional_dag/libutils.cpp)
target_include_directories(functional_dag PRIVATE "include/" ${JSONCPP_INCLUDE_DIRS})
target_link_directories(functional_dag PRIVATE ${JSONCPP_LIBRARY_DIRS})
target_link_libraries(functional_dag PRIVATE ${JSONCPP_LIBRARIES})

########################################
####### Build the testing files ########
########################################
if (DEFINED Catch2_DIR)
  add_executable(tests test/functional_dag/dag_tests.cpp 
                       test/functional_dag/lib_tests.cpp)
  target_include_directories(tests PRIVATE "include/" ${CATCH_INCLUDE_DIR})
  target_link_directories(tests PRIVATE ${CATCH_LIBRARY_DIRS} ${JSONCPP_LIBRARY_DIRS})
  target_link_libraries(tests functional_dag Catch2::Catch2WithMain ${JSONCPP_LIBRARIES})

  include(CTest)
  include(Catch)
  catch_discover_tests(tests)
endif()

########################################
####### Install specification ##########
########################################
target_sources(functional_dag INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_interface.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/dag_interface.hpp>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/lib_utils.h>
    $<INSTALL_INTERFACE:include/functional_dag/lib_utils.h>
    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_impl.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/dag_impl.hpp>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/filter_sys.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/filter_sys.hpp>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_fanout_impl.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/dag_fanout_impl.hpp>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_node_impl.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/dag_node_impl.hpp>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dlpack.h>
    $<INSTALL_INTERFACE:include/functional_dag/dlpack.h>
    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_utils.hpp>
    $<INSTALL_INTERFACE:include/functional_dag/dag_utils.hpp>)

get_target_property(FN_DAG_PUBLIC_HEADERS functional_dag INTERFACE_SOURCES)
set_target_properties(functional_dag PROPERTIES PUBLIC_HEADER "${FN_DAG_PUBLIC_HEADERS}")

install(TARGETS functional_dag 
    EXPORT dag_install
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/functional_dag)

install(EXPORT dag_install
    FILE functional_dagConfig.cmake
    NAMESPACE FunctionalDag::
    DESTINATION lib/cmake/functional_dag)