add_executable(EngineTests
    "TestHelpers.h"
    "CameraTests.cpp"
    "TransformTests.cpp"
    "PhysicsTests.cpp"
    "ModelLoadingTests.cpp"
    "UtilsTests.cpp"
)

target_link_libraries(EngineTests PRIVATE EngineCore GTest::gtest_main)

# Small hand-made meshes used by the model loading tests
target_compile_definitions(EngineTests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data/")

if(MSVC)
    # The tests never call into Vulkan, so only load vulkan-1.dll if something does.
    # This lets them run on CI machines that have no GPU driver installed.
    target_link_options(EngineTests PRIVATE "/DELAYLOAD:vulkan-1.dll")
    target_link_libraries(EngineTests PRIVATE delayimp)
endif()

# Run from Project/ so the tests open scene and model files with the same relative paths as the game
set(TESTS_WORKING_DIR "${PROJECT_SOURCE_DIR}/Project")
set_target_properties(EngineTests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${TESTS_WORKING_DIR}")

include(GoogleTest)
gtest_discover_tests(EngineTests WORKING_DIRECTORY "${TESTS_WORKING_DIR}")
