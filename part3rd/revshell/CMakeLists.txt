# copy from `c-reverse-shell-main`
# A reverse shell for Windows and Linux written in C.
set(REVSHELL_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(REVSHELL_BUILD_STATIC_LIBS "Build reverse-shell as a static library." ON)

set(REVSHELL_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/revshell"
)

# select source codes
file(GLOB REVSHELL_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/revshell/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/revshell/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/revshell/*.cpp
)

if(REVSHELL_BUILD_STATIC_LIBS)
    add_library(revshell STATIC ${REVSHELL_SOURCES})
    include_directories(${REVSHELL_INCLUDE_DIRS})
    target_compile_definitions(revshell PUBLIC ${COMPILE_DEFINITIONS})
    target_compile_options(revshell PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_link_libraries(revshell ws2_32)
endif() 


# target_link_libraries(drogon jsoncppstatic)
set_property(TARGET revshell APPEND PROPERTY INCLUDE_DIRECTORIES ${REVSHELL_INCLUDE_DIRS})
set_property(TARGET revshell APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${REVSHELL_INCLUDE_DIRS}>")

