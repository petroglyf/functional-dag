cmake_minimum_required(VERSION 3.6)
project(functional-dag LANGUAGES CXX )

#############################################
####### Setting up compiler options #########
#############################################
include(FindPkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories("include/")
add_compile_options("-stdlib=libc++" "-Wall" "-Wextra" "-Wpedantic" "-Werror")

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
add_library(functional_dag SHARED src/functional_dag/libutils.cpp)
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
####### Lint command (optional) ########
########################################
add_custom_target(lint
  COMMAND clang-tidy src/functional_dag/libutils.cpp -- -Iinclude/ -I${CMAKE_OSX_SYSROOT}/usr/include
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Linting the source"
  VERBATIM
)

########################################
####### Install specification ##########
########################################
set(HEADERS_ROOT_INSTALL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dag_interface.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/lib_utils.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/filter_sys.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/dlpack.h)

set(HEADERS_IMPL_INSTALL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/impl/dag_impl.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/impl/dag_fanout_impl.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/impl/dag_node_impl.hpp)
      
set(HEADERS_CORE_INSTALL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/functional_dag/core/dag_utils.hpp)

install(FILES ${HEADERS_ROOT_INSTALL} DESTINATION include/functional_dag)
install(FILES ${HEADERS_IMPL_INSTALL} DESTINATION include/functional_dag/impl)
install(FILES ${HEADERS_CORE_INSTALL} DESTINATION include/functional_dag/core)

install(TARGETS functional_dag 
    EXPORT dag_install
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT dag_install
    FILE functional_dagConfig.cmake
    NAMESPACE FunctionalDag::
    DESTINATION lib/cmake/functional_dag)