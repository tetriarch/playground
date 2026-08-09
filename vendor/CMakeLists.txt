add_library(lua::lua STATIC IMPORTED GLOBAL)
set_target_properties(lua::lua
    PROPERTIES IMPORTED_LOCATION 
    "${CMAKE_CURRENT_SOURCE_DIR}/lua/lib/liblua.a" 
    INTERFACE_INCLUDE_DIRECTORIES 
    "${CMAKE_CURRENT_SOURCE_DIR}/lua/include"
)

set(stb_SRCS "stb/stb_image.cpp" "stb/stb_image.h")
add_library(stb STATIC ${stb_SRCS})
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
