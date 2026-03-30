add_executable(TomsDataOnion 
    "main.cpp"

    "ascii85.cpp"
    "bitwise.cpp"
    "network.cpp"
)

add_custom_command(
        TARGET TomsDataOnion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/
                ${CMAKE_CURRENT_BINARY_DIR}/data/
)