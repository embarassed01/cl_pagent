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

# MacOS
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")  # for include <uuid.h>
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        include_directories(SYSTEM "/opt/homebrew/include/ossp")
    endif()
endif()

if(DROGON_BUILD_STATIC_LIBS)
    set(DROGON_LIBRARIES drogon)
    add_library(drogon STATIC ${DROGON_SOURCES})
    include_directories(${DROGON_INCLUDE_DIRS})
    target_compile_definitions(drogon PUBLIC ${COMPILE_DEFINITIONS})
    target_compile_options(drogon PRIVATE ${COMPILE_OPTIONS_DEFAULT})
endif() 

set_property(TARGET drogon PROPERTY CXX_STANDARD 20)
set_property(TARGET drogon PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET drogon PROPERTY CXX_EXTENSIONS OFF)

target_link_libraries(drogon trantor zlibstatic jsoncppstatic brotliencstatic brotlidecstatic brotlicommonstatic)
set_property(TARGET drogon APPEND PROPERTY INCLUDE_DIRECTORIES ${DROGON_INCLUDE_DIRS})
set_property(TARGET drogon APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${DROGON_INCLUDE_DIRS}>")
