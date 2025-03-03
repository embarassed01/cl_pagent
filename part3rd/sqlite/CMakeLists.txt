set(SQLITE3_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(SQLITE3_BUILD_SHARED_LIBS "Build sqlite3 as a shared library." ON)
option(SQLITE3_BUILD_STATIC_LIBS "Build sqlite3 as a static library." ON)

file(GLOB SQLITE3_SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3ext.h 
)

if(SQLITE3_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(SQLITE3_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/art/sqlite3.rc)
    endif()
    # Parse the full version number from sqlite3.h and include in SQLITE3_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.h _sqlite3_h_contents)
    string(REGEX REPLACE ".*#define[ \t]+SQLITE_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*" "\\1" SQLITE3_FULL_VERSION ${_sqlite3_h_contents})
    # message(STATUS "sqlite3_full_version: ${SQLITE3_FULL_VERSION}")  # 3.48.0
    if(MINGW)
        # This gets us DLL resource information when compiling on MinGW.
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres.exe)
        endif()
        add_custom_command(OUTPUT ${SQLITE3_LIBRARY_OUTPUT_PATH}/sqlite3.obj 
                COMMAND ${CMAKE_RC_COMPILER}
                -D GCC_WINDRES
                -I ${CMAKE_CURRENT_SOURCE_DIR}/
                -I ${SQLITE3_LIBRARY_OUTPUT_PATH}
                -o ${SQLITE3_LIBRARY_OUTPUT_PATH}/sqlite3.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/art/sqlite3.rc
        )
        set(SQLITE3_DLL_SRCS ${SQLITE3_LIBRARY_OUTPUT_PATH}/sqlite3.obj)
    endif(MINGW)
endif()

if(SQLITE3_BUILD_SHARED_LIBS)
    add_library(sqlite3 SHARED ${SQLITE3_DLL_SRCS} ${SQLITE3_SRC_LIST})
    target_compile_options(sqlite3 PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(sqlite3 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/"
    )
    set_target_properties(sqlite3 PROPERTIES DEFINE_SYMBOL SQLITE3_DLL)
    set_target_properties(sqlite3 PROPERTIES SOVERSION 1)
    if(NOT CYGWIN)  # togo
        set_target_properties(sqlite3 PROPERTIES VERSION ${SQLITE3_FULL_VERSION})
    endif()
endif()

if(SQLITE3_BUILD_STATIC_LIBS)
    add_library(sqlite3static STATIC ${SQLITE3_SRC_LIST})
    target_compile_options(sqlite3static PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(sqlite3static 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/"
    )
    # update_cached_list(PA_LINK_LIBRARIES sqlite3static)
endif()