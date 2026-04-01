add_executable(TomsDataOnion 
    "main.cpp"

    "aes.cpp"
    "ascii85.cpp"
    "bitwise.cpp"
    "network.cpp"
    "virtual_machine.cpp"
)

target_link_libraries(TomsDataOnion
    PRIVATE
        OpenSSL::Crypto
)

add_custom_command(
        TARGET TomsDataOnion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/
                ${CMAKE_CURRENT_BINARY_DIR}/data/
)