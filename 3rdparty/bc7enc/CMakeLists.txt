add_library(bc7enc
	include/bc7decomp.h
	include/rgbcx.h
	src/bc7decomp.cpp
	src/rgbcx.cpp
	src/rgbcx_table4_small.h
)

target_include_directories(bc7enc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(bc7enc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(bc7enc PRIVATE "RGBCX_USE_SMALLER_TABLES")
set_property(TARGET bc7enc PROPERTY CXX_STANDARD 17)

