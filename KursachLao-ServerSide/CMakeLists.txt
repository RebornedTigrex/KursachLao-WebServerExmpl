cmake_minimum_required(VERSION 3.16)
project(HRSystemMVP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Release (в продакшене) — оптимизация и без дебага
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Boost 1.89 REQUIRED)
# find_package(Threads REQUIRED)

# Добавляем все .cpp из src
file(GLOB_RECURSIVE SOURCES "src/*.cpp")

add_executable(hr-server ${SOURCES})

target_include_directories(hr-server PRIVATE src)
target_link_libraries(hr-server 
   
)

find_package(Boost REQUIRED COMPONENTS beast, program_options, asio)
target_link_libraries(main PRIVATE 
    Boost::program_options
    Boost::beast
    Boost::asio
)

# Копируем папку static рядом с бинарником
add_custom_command(TARGET hr-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/static $<TARGET_FILE_DIR:hr-server>/static
)