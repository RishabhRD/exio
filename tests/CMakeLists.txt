file(GLOB_RECURSE test_list "*_test.cpp")
file(GLOB_RECURSE src_list "../src/exio/*.cpp")
add_executable(exio-tests ${src_list} ${test_list} main.cpp)
target_include_directories(exio-tests SYSTEM PRIVATE ../lib)
target_include_directories(exio-tests PRIVATE ../include)
target_link_libraries(exio-tests PRIVATE
  project_options
  project_warnings
  doctest::doctest
)
