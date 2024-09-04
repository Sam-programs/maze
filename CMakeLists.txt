cmake_minimum_required(VERSION 3.12)

project(unnamed)

file(GLOB SRC src/*.c*)
add_executable(main ${SRC})

set(ASAN -fsanitize=address -fsanitize=undefined)
set(CXXFLAGS 
-g
-Wall -Wextra -Werror -pedantic 
-Wno-unused-parameter -Wno-error=unused-variable 
-Wno-missing-field-initializers
${ASAN})
set(LIBS raylib)
set(LDFLAGS ${ASAN})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_link_libraries(main ${LIBS} ${LDFLAGS})
target_compile_options(main PUBLIC ${CXXFLAGS})
