project(ui C)

# UI
file(GLOB UI_SOURCES ${CMAKE_SOURCE_DIR}/game/bg_misc.c ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.def)
list(REMOVE_ITEM UI_SOURCES
	${CMAKE_SOURCE_DIR}/ui/ui_util.c
	${CMAKE_SOURCE_DIR}/ui/ui_specifyleague.c
	${CMAKE_SOURCE_DIR}/ui/ui_signup.c
	${CMAKE_SOURCE_DIR}/ui/ui_login.c
	${CMAKE_SOURCE_DIR}/ui/ui_shared.c
	${CMAKE_SOURCE_DIR}/ui/ui_rankings.c
	${CMAKE_SOURCE_DIR}/ui/ui_rankstatus.c)
source_group( "UI" FILES ${UI_SOURCES} )
file(GLOB UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM UI_HEADERS ${CMAKE_SOURCE_DIR}/ui/ui_shared.h)
source_group( "UI" FILES ${UI_HEADERS} )

if (NOT HARDLINK_GAME)
	add_library(${PROJECT_NAME} SHARED ${UI_SOURCES} ${UI_HEADERS})
	target_link_libraries(${PROJECT_NAME} PRIVATE qcommon)
else()
	add_library(${PROJECT_NAME} STATIC ${UI_SOURCES} ${UI_HEADERS})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE UI_EXPORTS)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "uix86")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER})