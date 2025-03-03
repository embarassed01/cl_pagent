set(JSONCPP_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(JSONCPP_BUILD_SHARED_LIBS "Build jsoncpp as a shared library." ON)
option(JSONCPP_BUILD_STATIC_LIBS "Build jsoncpp as a static library." ON)

file(GLOB JSONCPP_SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/json/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib_json/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib_json/*.h 
)

if(JSONCPP_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(JSONCPP_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp.rc)
    endif()
    # Parse the full version number from version.h and include in JSONCPP_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/json/version.h _jsoncpp_version_h_contents)
    string(REGEX REPLACE ".*#define[ \t]+JSONCPP_VERSION_STRING[ \t]+\"([-0-9A-Za-z.]+)\".*" "\\1" JSONCPP_FULL_VERSION ${_jsoncpp_version_h_contents})
    # message(STATUS "jsoncpp_full_version: ${JSONCPP_FULL_VERSION}")  # 1.9.6
    if(MINGW)
        # This gets us DLL resource information when compiling on MinGW.
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres.exe)
        endif()
        add_custom_command(OUTPUT ${JSONCPP_LIBRARY_OUTPUT_PATH}/jsoncpp.obj 
                COMMAND ${CMAKE_RC_COMPILER}
                -D GCC_WINDRES
                -I ${CMAKE_CURRENT_SOURCE_DIR}/
                -I ${CMAKE_CURRENT_SOURCE_DIR}/include
                -I ${JSONCPP_LIBRARY_OUTPUT_PATH}
                -o ${JSONCPP_LIBRARY_OUTPUT_PATH}/jsoncpp.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp.rc
        )
        set(JSONCPP_DLL_SRCS ${JSONCPP_LIBRARY_OUTPUT_PATH}/jsoncpp.obj)
    endif(MINGW)
endif()
if(JSONCPP_BUILD_SHARED_LIBS)
    add_library(jsoncpp SHARED ${JSONCPP_DLL_SRCS} ${JSONCPP_SRC_LIST})
    target_compile_options(jsoncpp PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(jsoncpp 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
    )
    set_target_properties(jsoncpp PROPERTIES DEFINE_SYMBOL JSONCPP_DLL)
    set_target_properties(jsoncpp PROPERTIES SOVERSION 26)
endif()

if(JSONCPP_BUILD_STATIC_LIBS)
    add_library(jsoncppstatic STATIC ${JSONCPP_SRC_LIST})
    target_compile_options(jsoncppstatic PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(jsoncppstatic 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
    )
    # update_cached_list(PA_LINK_LIBRARIES jsoncppstatic)
endif()