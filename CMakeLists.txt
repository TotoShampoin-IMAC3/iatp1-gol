cmake_minimum_required(VERSION 3.27) # I wanna go higher, but clangd doesn't seem to support modules
project(ai-tp)

include("utils/files_and_folders.cmake")
include(vendors/imgui.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/dist)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE INC_FILES includes/*.h includes/*.hpp)

add_subdirectory(vendors/glad)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)


add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE includes)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL glfw glad glm imgui)

Cool__target_copy_folder(${PROJECT_NAME} "resources")
