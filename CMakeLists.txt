cmake_minimum_required(VERSION 3.15)
project(user_registry LANGUAGES C CXX)  # C is needed for civetweb.c

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Paths ----
set(CIVETWEB_ROOT "${CMAKE_SOURCE_DIR}/civetweb")

# ---- Our sources ----
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")

# ---- Executable: include CivetWeb C core + C++ wrapper directly ----
add_executable(user_registry
    ${SRC_FILES}
    ${CIVETWEB_ROOT}/src/civetweb.c
    ${CIVETWEB_ROOT}/src/CivetServer.cpp
)
add_executable(gen_users gen_users.cpp)
target_link_libraries(gen_users PRIVATE PostgreSQL::PostgreSQL)


# Include dirs
target_include_directories(user_registry PRIVATE
    src
    ${CIVETWEB_ROOT}/include
    ${CIVETWEB_ROOT}/include/civetweb
)

# CivetWeb feature flags (must match for both sources)
target_compile_definitions(user_registry PRIVATE
    NO_FILES          # don't serve static files
    USE_WEBSOCKET=1   # CivetServer.cpp references websocket symbols
    NO_SSL=1          # <-- disable CivetWeb TLS to avoid OpenSSL API macro issues
    # USE_IPV6=1
)

# ---- Dependencies ----
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)

message(STATUS "PostgreSQL include dirs: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "PostgreSQL libraries   : ${PostgreSQL_LIBRARIES}")

target_include_directories(user_registry PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(user_registry PRIVATE
    PostgreSQL::PostgreSQL
    OpenSSL::SSL            # keep for PBKDF2 (we use OpenSSL directly)
    OpenSSL::Crypto
    Threads::Threads
)

if(UNIX AND NOT APPLE)
    target_link_libraries(user_registry PRIVATE dl)
endif()
