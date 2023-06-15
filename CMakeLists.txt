cmake_minimum_required(VERSION 3.10)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
include_directories(${Boost_INCLUDE_DIR})

project(bayan VERSION ${PROJECT_VESRION})

find_package(Boost COMPONENTS filesystem)

configure_file(version.h.in version.h)

add_executable(bayan bayan.cpp)

set_target_properties(bayan PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(bayan
    PRIVATE "${CMAKE_BINARY_DIR}"
)

target_link_libraries(bayan
    ${Boost_FILESYSTEM_LIBRARY}
)

if (MSVC)
    target_compile_options(bayan PRIVATE
        /W4
    )
else ()

    target_compile_options(bayan PRIVATE
        -Wall -Wextra -pedantic -Werror
    )

endif()



install(TARGETS bayan RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
