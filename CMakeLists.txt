cmake_minimum_required(VERSION 3.10) # cmake version
project(networking) # project name
cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g -O0)# add_subdirectory(libs/Mylibs) # add Mylibs library
find_package(Boost 1.89 REQUIRED)  # No COMPONENTS
add_executable(networking src/main.cpp) # target which need to be execute
add_executable(client src/net_client.cpp)
add_executable(server src/net_server.cpp)
# target_include_directories(networking PUBLIC libs/Mylibs) # target include directory of Mylibs library
target_include_directories(networking PUBLIC header_files)
target_include_directories(client PRIVATE header_files)
target_include_directories(server PRIVATE header_files)
link_directories(/opt/homebrew/lib)
target_link_libraries(networking
    Boost::boost         # header-only
)
target_link_libraries(client
    Boost::boost         # header-only
)
target_link_libraries(server
    Boost::boost         # header-only
)

