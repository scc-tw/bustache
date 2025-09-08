# Examples for bustache
cmake_minimum_required(VERSION 3.15)

# Demo example
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE bustache)

# Boost.Describe example (optional, only if Boost is available)
find_package(Boost QUIET)
if(Boost_FOUND)
    add_executable(boost_describe_example boost_describe.cpp)
    target_link_libraries(boost_describe_example PRIVATE bustache Boost::boost)
endif()

# nlohmann::json example (optional, only if nlohmann_json is available)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    add_executable(nlohmann_example nlohmann.cpp)
    target_link_libraries(nlohmann_example PRIVATE bustache nlohmann_json::nlohmann_json)
endif()
