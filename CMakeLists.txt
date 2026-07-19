cmake_minimum_required(VERSION 3.22.0)

set(VERSION "0.0.1")
set(PROJECT_NAME "playground")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

include(FetchContent)

# FetchDependency to avoid re-compilation of deps on every build execution
FetchContent_Declare(FetchDependency
    GIT_REPOSITORY https://github.com/jpetrie/fetch-dependency.git
    GIT_TAG origin/main
)
FetchContent_MakeAvailable(FetchDependency)
include(${fetchdependency_SOURCE_DIR}/FetchDependency.cmake)

# SDL3
fetch_dependency(
        SDL3
        GIT_SOURCE https://github.com/libsdl-org/SDL.git
        VERSION release-3.2.20
        CONFIGURATION ${CMAKE_BUILD_TYPE}
        GENERATE_OPTIONS -DSDL_SHARED=OFF -DSDL_STATIC=ON -DBUILD_SHARED_LIBS=OFF
)

# Sol2 
fetch_dependency(Sol2
    GIT_SOURCE https://github.com/ThePhD/sol2.git
    VERSION v3.5.0
    CONFIGURATION ${CMAKE_BUILD_TYPE}
)

# GLM
fetch_dependency(glm
        GIT_SOURCE https://github.com/g-truc/glm.git
        VERSION 1.0.1
        CONFIGURATION ${CMAKE_BUILD_TYPE}
        CONFIGURE_OPTIONS -DGLM_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
)


add_subdirectory(vendor)
file(GLOB_RECURSE playground_files CONFIGURE_DEPENDS "src/**.cpp")
add_executable(${PROJECT_NAME} ${playground_files})

# compile flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -m64
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wno-unused-parameter
)

# compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
    DEBUG
    >
    $<$<CONFIG:Release>:
    NDEBUG
    >
)

target_precompile_headers(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_DIR}/src/tengine/pch.hpp>)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE lua::lua sol2::sol2 glm::glm)

# link compile_commands.json
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
)

add_custom_target(
    copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

add_dependencies(${PROJECT_NAME} copy_assets)
