#Add debug compile
cmake_minimum_required(VERSION 3.16)
project(tbb_demo DESCRIPTION "small show case for threading building block" LANGUAGES CXX)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(SRC_FILES test_tbb_main.cc tbb_test.cc)

#BOOST setup
find_package(TBB REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

#Create unit test exe for the connectivity_extractor
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::unit_test_framework TBB::tbb  TBB::tbbmalloc)

