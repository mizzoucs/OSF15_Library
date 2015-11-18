cmake_minimum_required (VERSION 2.8)
project(c_map)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -O3")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include_directories(${dyn_array_INCLUDE_DIRS})
add_library(${PROJECT_NAME} SHARED src/${PROJECT_NAME}.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} dyn_array)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES include/${PROJECT_NAME}.h DESTINATION include)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
	CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

