include(FetchContent)

message(CHECK_START "Fetching GTest")
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF)
set(GTEST_HAS_ABSL OFF)
FetchContent_MakeAvailable(googletest)

add_executable(TomsDataOnionTest 
    "test_main.cpp"  
    
    "${PROJECT_SOURCE_DIR}/src/ascii85.cpp"
    "${PROJECT_SOURCE_DIR}/src/bitwise.cpp"
    "${PROJECT_SOURCE_DIR}/src/network.cpp"
    "${PROJECT_SOURCE_DIR}/src/virtual_machine.cpp"

    "test_ascii85.cpp"
    "test_bitwise.cpp"
    "test_network.cpp"
    "test_virtual_machine.cpp"
)

target_include_directories(TomsDataOnionTest
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(TomsDataOnionTest 
    PRIVATE
        gtest_main
)
