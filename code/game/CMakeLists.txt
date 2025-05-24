project(game CXX)

# GAME
file(GLOB GAME_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.def)
list(REMOVE_ITEM GAME_SOURCES ${CMAKE_SOURCE_DIR}/game/bg_lib.cpp ${CMAKE_SOURCE_DIR}/game/g_rankings.cpp)
source_group( "Game" FILES ${GAME_SOURCES} )
file(GLOB GAME_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM GAME_HEADERS ${CMAKE_SOURCE_DIR}/game/bg_lib.h)
source_group( "Game" FILES ${GAME_HEADERS} )

if (NOT HARDLINK_GAME)
	add_library(${PROJECT_NAME} SHARED ${GAME_SOURCES} ${GAME_HEADERS})
	target_link_libraries(${PROJECT_NAME} PRIVATE qcommon)
else()
	add_library(${PROJECT_NAME} STATIC ${GAME_SOURCES} ${GAME_HEADERS})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE QAGAME)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "qagamex86")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER})