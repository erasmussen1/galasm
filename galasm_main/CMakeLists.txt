project(galasm)

add_executable(${PROJECT_NAME} galasm_main.cpp)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC galasm2)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR} )
