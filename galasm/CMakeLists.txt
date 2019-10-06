project(galasm2)

add_library(${PROJECT_NAME} ${LIBRARY_TYPE}
    src/galasm2.c
    src/galasm.c
    src/jedec.c
    src/localize.c
    src/support.c
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# target_link_libraries(${PROJECT_NAME} LINK_PUBLIC galasm2)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_LIB_DIR} )

if (BUILD_UNITTEST)
  add_subdirectory(test)
endif()

