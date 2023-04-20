cmake_minimum_required(VERSION 3.6)
project(vin LANGUAGES CXX )

################################
####### PREP FOR TORCH #########
################################

# Make sure conda is activated first.
if(NOT DEFINED ENV{CONDA_PREFIX})
  message(FATAL_ERROR " Conda must be activated for this build to proceed.")
  return()
else()
  set(_python_exe $ENV{CONDA_PREFIX}/bin/python)
endif ()

# Use conda's python to get the site packages directory
function(getsitepackages outvar)
  set(_fixup_cmd "import sys;print(list(filter(lambda x: x.endswith('site-packages'), sys.path))[0])")
  execute_process(
    COMMAND "${_python_exe}" -c "${_fixup_cmd}"
    RESULT_VARIABLE _getpath_exitcode
    OUTPUT_VARIABLE _getpath_text)
  if(NOT _getpath_exitcode EQUAL 0)
    message(ERROR "Failed to get the cmake path for torch. Are you sure torch is installed for python?")
    message(FATAL_ERROR " Python site-packages failed with error code: ${_getpath_exitcode}")
  endif()
  string(STRIP "${_getpath_text}" _path_stripped)
  set(${outvar} "${_path_stripped}" PARENT_SCOPE)
endfunction()

getsitepackages(PYTHON_SITE_PACKAGES)

# Now use the site packages to add some custom cmake directories
set(_TORCH_CMAKE_DIR "${PYTHON_SITE_PACKAGES}/torch/share/cmake")
set(CMAKE_PREFIX_PATH "${_TORCH_CMAKE_DIR}")


#############################################
####### Setting up compiler options #########
#############################################
#### bin2c for models during build
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
find_package(Torch REQUIRED)
find_package(Catch2 3 REQUIRED)
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


########################################
####### Building specification #########
########################################
add_library(filterdag STATIC src/filter_sys/dag_manager.cpp src/filter_sys/libutils.cpp)
target_include_directories(filterdag PRIVATE "include/" ${TORCH_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})


########################################
####### Build the testing files ########
########################################
if (BUILD_TESTING)
  message("HELLO")
  add_executable(tests test/filter_sys/dag_tests.cpp test/filter_sys/lib_tests.cpp)
  target_include_directories(tests PRIVATE "include/" ${CATCH_INCLUDE_DIR} ${TORCH_INCLUDE_DIRS})
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
    include/filter_sys/lib_utils.h
    include/filter_sys/dag_utils.hpp)

install(TARGETS filterdag 
    EXPORT filterdagTargets
    FILE_SET HEADERS
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

install(EXPORT filterdagTargets
    FILE FilterDag.cmake
    NAMESPACE FilterDag::
    DESTINATION lib/cmake/filterdag)