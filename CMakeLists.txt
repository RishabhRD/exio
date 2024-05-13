cmake_minimum_required(VERSION 3.20)
project(exio CXX)
include(cmake/StandardProjectSettings.cmake)
# include(cmake/StaticAnalyzers.cmake)
include(cmake/CompilerWarnings.cmake)
include(package.cmake)

set(CMAKE_CXX_STANDARD 20)
# add_compile_options(-fconcepts-diagnostics-depth=2)

add_library(project_options INTERFACE)

add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

add_subdirectory(examples)
