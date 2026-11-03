cmake_minimum_required(VERSION 3.10)

# Project Name
project(VulkanLab01)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8 # Specify the version you need
  GIT_SHALLOW TRUE
)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

FetchContent_MakeAvailable(glfw)

# Find the required packages
find_package(Vulkan REQUIRED)

# Include Directories
include_directories(${Vulkan_INCLUDE_DIRS})

# GLM is an optional part of the Vulkan SDK, so download it if the SDK doesn't have it
find_path(GLM_INCLUDE_DIR glm/glm.hpp HINTS ${Vulkan_INCLUDE_DIRS})
if(NOT GLM_INCLUDE_DIR)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW TRUE
  )
  option(GLM_BUILD_LIBRARY "Build dynamic/static library" OFF)

  FetchContent_MakeAvailable(glm)
  include_directories(${glm_SOURCE_DIR})
endif()

add_subdirectory(Project)

# Unit tests (GoogleTest + CTest)
option(BUILD_TESTS "Build the unit tests" ON)

if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW TRUE
  )
  # Link the same C runtime as the engine on MSVC, otherwise the linker fails
  option(gtest_force_shared_crt "Use shared (DLL) run-time lib even when Google Test is built as static lib." ON)
  option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
  option(INSTALL_GTEST "Enable installation of googletest." OFF)

  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_subdirectory(Tests)
endif()


# If using validation layers, copy the required JSON files (optional)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/path_to_json/VK_LAYER_KHRONOS_validation.json"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
