cmake_minimum_required(VERSION 3.15)
project(user_registry LANGUAGES C CXX) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CIVETWEB_ROOT "${CMAKE_SOURCE_DIR}/civetweb")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(user_registry
    ${SRC_FILES}
    ${CIVETWEB_ROOT}/src/civetweb.c
    ${CIVETWEB_ROOT}/src/CivetServer.cpp
)
add_executable(gen_users gen_users.cpp)
target_link_libraries(gen_users PRIVATE PostgreSQL::PostgreSQL)


target_include_directories(user_registry PRIVATE
    src
    ${CIVETWEB_ROOT}/include
    ${CIVETWEB_ROOT}/include/civetweb
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(user_registry PRIVATE
    NO_FILES
    USE_WEBSOCKET=1
    NO_SSL=1
)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

message(STATUS "PostgreSQL include dirs: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "PostgreSQL libraries : ${PostgreSQL_LIBRARIES}")

target_include_directories(user_registry PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(user_registry PRIVATE
    PostgreSQL::PostgreSQL
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    nlohmann_json::nlohmann_json
)

if(UNIX AND NOT APPLE)
    target_link_libraries(user_registry PRIVATE dl)
endif()
