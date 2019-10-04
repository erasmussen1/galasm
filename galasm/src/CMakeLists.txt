project(galasm)

add_executable(${PROJECT_NAME}
    galasm.c
    jedec.c
    localize.c
    support.c
)

# target_link_libraries(${PROJECT_NAME} LINK_PUBLIC crc)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_BIN_DIR} )
