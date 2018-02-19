cmake_minimum_required(VERSION 3.1.0)

get_filename_component(ProjectName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${ProjectName})

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Werror -Wcast-qual -Wconversion -Wunreachable-code -Wmissing-noreturn -Wshadow -Wfatal-errors")

file(GLOB_RECURSE SRC cpp/*.cpp)
include_directories(cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} stdc++fs)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

file(GLOB CLANG_FORMAT_FILE .clang-format)

find_program(CLANG_FORMAT clang-format) 
if(EXISTS ${CLANG_FORMAT})
  add_custom_target(clangformat
        COMMAND clang-format
        -style=file
        -assume-filename=${CLANG_FORMAT_FILE}
        -i
        ${SRC}
  )

  add_dependencies(${PROJECT_NAME} clangformat)
else()
  message(AUTHOR_WARNING "clang-format is not installed, auto formating will not be performed")
endif()




