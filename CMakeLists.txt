#Add debug compile
cmake_minimum_required(VERSION 3.16)

# Add project_options v0.24.1
# https://github.com/aminya/project_options
# Change the version in the following URL to update the package (watch the releases of the repository for future updates)
include(FetchContent)
FetchContent_Declare(_project_options URL https://github.com/aminya/project_options/archive/refs/tags/v0.24.1.zip)
FetchContent_MakeAvailable(_project_options)
include(${_project_options_SOURCE_DIR}/Index.cmake)

# install vcpkg dependencies: - should be called before defining project()
#run_vcpkg()


project(tbb_demo DESCRIPTION "small show case for threading building block" LANGUAGES CXX)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Initialize project_options variable related to this project
# This overwrites `project_options` and sets `project_warnings`
# uncomment to enable the options. Some of them accept one or more inputs:
project_options(
        ENABLE_CACHE
        ${ENABLE_CPPCHECK}
        ${ENABLE_CLANG_TIDY}
        ENABLE_VS_ANALYSIS
        # ENABLE_CONAN
        # ENABLE_INTERPROCEDURAL_OPTIMIZATION
        # ENABLE_NATIVE_OPTIMIZATION
        ${ENABLE_DOXYGEN}
        ${ENABLE_COVERAGE}
        ${ENABLE_SANITIZER_ADDRESS}
        ${ENABLE_SANITIZER_UNDEFINED_BEHAVIOR}
        # ENABLE_SANITIZER_THREAD
        # ENABLE_SANITIZER_MEMORY
        # ENABLE_PCH
        # PCH_HEADERS
        # WARNINGS_AS_ERRORS
        # ENABLE_INCLUDE_WHAT_YOU_USE
        # ENABLE_USER_LINKER
        # ENABLE_BUILD_WITH_TIME_TRACE
        # ENABLE_UNITY
        # CONAN_PROFILE ${profile_path}  # passes a profile to conan: see https://docs.conan.io/en/latest/reference/profiles.html
)

set(SRC_FILES test_tbb_main.cc
              tbb_test.cc)

#BOOST setup
find_package(TBB REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


#Create unit test exe for the connectivity_extractor
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE project_options project_warnings
        Boost::unit_test_framework
        TBB::tbb
        TBB::tbbmalloc
        Threads::Threads
        )

