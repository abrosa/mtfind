# actually it doesn't matter
cmake_minimum_required(VERSION 3.24)

# project name
project(mtfind VERSION 1.0)

# add executable
add_executable(mtfind mtfind.cpp)

# actually it doesn't matter
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_REQUIRED_STANDARD True)

# target include directories
target_include_directories(mtfind PUBLIC ${PROJECT_BINARY_DIR})

# set boost directories
set(Boost_INCLUDE_DIR C:/msys64/mingw64/include)
set(Boost_LIBRARY_DIR C:/msys64/mingw64/lib)

# find boost package
find_package(Boost 1.80 COMPONENTS iostreams REQUIRED)

# add boost to include directories
include_directories(${Boost_INCLUDE_DIR})

# add boost to link directories
link_directories(${Boost_LIBRARY_DIR})

# link boost to application
target_link_libraries(mtfind ${Boost_IOSTREAMS_LIBRARY})
