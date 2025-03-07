set(DROGON_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(DROGON_BUILD_STATIC_LIBS "Build drogon as a static library." ON)

set(DROGON_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

# select source codes
file(GLOB DROGON_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

if(DROGON_BUILD_STATIC_LIBS)
    set(DROGON_LIBRARIES drogon)
    add_library(drogon STATIC ${DROGON_SOURCES})
    include_directories(${DROGON_INCLUDE_DIRS})
    target_compile_definitions(drogon PUBLIC ${COMPILE_DEFINITIONS})
    target_compile_options(drogon PRIVATE ${COMPILE_OPTIONS_DEFAULT})
endif() 

target_link_libraries(drogon jsoncppstatic)
set_property(TARGET drogon APPEND PROPERTY INCLUDE_DIRECTORIES ${DROGON_INCLUDE_DIRS})
set_property(TARGET drogon APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${DROGON_INCLUDE_DIRS}>")
