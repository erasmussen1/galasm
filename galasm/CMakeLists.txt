project(galasm2 VERSION 2.1.1)

add_library(${PROJECT_NAME} ${BUILD_TYPE}
    src/galasm2.c
    src/galasm.c
    src/jedec.c
    src/localize.c
    src/support.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
     SOVERSION ${PROJECT_VERSION_MAJOR}
     VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersions.cmake"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    COMPATIBILITY SameMajorVersion
)

# target_link_libraries(${PROJECT_NAME} LINK_PUBLIC galasm2)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_LIB_DIR} )

add_subdirectory(test)

