cmake_minimum_required(VERSION 3.10)
project(echobot)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Boost multithreading
set(Boost_USE_MULTITHREADED ON)

# If using custom Boost:
# set(BOOST_ROOT "/opt/boost_1_83_0")
# set(CMAKE_PREFIX_PATH "/opt/boost_1_83_0")

# Find dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.83.0 REQUIRED COMPONENTS system)
find_package(CURL REQUIRED)

# Include headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include      # If TgBot headers are here
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Define executable
add_executable(echobot src/main.cpp)


target_link_libraries(echobot
    /home/cnc/Desktop/MY_GIT/Tele_bot/lib/libTgBot.a
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ssl
    crypto
)
