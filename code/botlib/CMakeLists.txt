project(botlib C)

# BotLib
file(GLOB BOTLIB_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
source_group( "BotLib" FILES ${CLIENT_SOURCES} )
file(GLOB BOTLIB_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group( "BotLib" FILES ${BOTLIB_HEADERS} )

add_library(${PROJECT_NAME} STATIC ${BOTLIB_SOURCES} ${BOTLIB_HEADERS})
target_compile_definitions(${PROJECT_NAME} PRIVATE BOTLIB)