#Add debug compile
cmake_minimum_required(VERSION 3.16)
project(tbb_tests DESCRIPTION "small show case for Threading Building Block" LANGUAGES CXX)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(SRC_FILES test_tbb_main.cc tbb_test.cc)

#BOOST setup
set(BOOST_ROOT /prj/qct/sve/dragonr/tools/boost_1_73_0/)
set(Boost_USE_STATIC_LIBS  ON)
find_package(Boost 1.73.0 REQUIRED COMPONENTS unit_test_framework)

# Configure TBB
set (TBB_DIR /prj/qct/sve/dragonr/tools/oneTBB/)
set (TBB_INCLUDE_DIR ${TBB_DIR}/include/)
set (TBB_LIB_DIR /prj/qct/sve/dragonr/tools/oneTBB/gnu_10.2_cxx11_64_relwithdebinfo/)

include_directories(${TBB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

#Create unit test exe for the connectivity_extractor
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${TBB_LIB_DIR}/libtbb.so ${TBB_LIB_DIR}/libtbbmalloc.so  )

