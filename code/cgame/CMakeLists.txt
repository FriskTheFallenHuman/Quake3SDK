project(cgame C)

# CGAME
file(GLOB CGAME_SOURCES ${CMAKE_SOURCE_DIR}/game/bg_misc.c ${CMAKE_SOURCE_DIR}/game/bg_pmove.c ${CMAKE_SOURCE_DIR}/game/bg_slidemove.c ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.def)
list(REMOVE_ITEM CGAME_SOURCES ${CMAKE_SOURCE_DIR}/cgame/cg_particles.c)
source_group( "CGame" FILES ${CGAME_SOURCES} )
file(GLOB CGAME_HEADERS ${CMAKE_SOURCE_DIR}/game/bg_public.h ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group( "CGame" FILES ${CGAME_HEADERS} )

if (NOT HARDLINK_GAME)
	add_library(${PROJECT_NAME} SHARED ${CGAME_SOURCES} ${CGAME_HEADERS})
	target_link_libraries(${PROJECT_NAME} PRIVATE qcommon)
else()
	add_library(${PROJECT_NAME} STATIC ${CGAME_SOURCES} ${CGAME_HEADERS})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE CGAME)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "cgamex86")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER})