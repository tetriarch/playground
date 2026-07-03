add_library(lua::lua STATIC IMPORTED GLOBAL)
set_target_properties(lua::lua
    PROPERTIES IMPORTED_LOCATION 
    "${CMAKE_CURRENT_SOURCE_DIR}/lua/lib/liblua.a" 
    INTERFACE_INCLUDE_DIRECTORIES 
    "${CMAKE_CURRENT_SOURCE_DIR}/lua/include"
)
