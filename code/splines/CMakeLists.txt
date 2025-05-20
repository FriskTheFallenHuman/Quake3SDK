project(splines CXX)

# Splines
file(GLOB SPLINES_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group( "Splines" FILES ${SPLINES_SOURCES} )
file(GLOB SPLINES_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group( "Splines" FILES ${SPLINES_HEADERS} )

add_library(${PROJECT_NAME} STATIC ${SPLINES_SOURCES} ${SPLINES_HEADERS})
target_compile_definitions(${PROJECT_NAME} PRIVATE SPLINES)