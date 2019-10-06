project(test_galasm)

add_executable(${PROJECT_NAME} test_galasm.cpp)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC galasm2 -lgtest -lgtest_main -lpthread)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/galasm/test/data/
                   ${CMAKE_BINARY_DIR}/galasm/test/
                   )

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR} )

