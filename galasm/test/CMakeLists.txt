project(test_galasm)

add_executable(${PROJECT_NAME} testdemo.cpp)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC galasm2 -lgtest -lgtest_main -lpthread)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR} )

