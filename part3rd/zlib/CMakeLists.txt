set(ZLIB_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(ZLIB_BUILD_SHARED_LIBS "Build zlib as a shared library." ON)
option(ZLIB_BUILD_STATIC_LIBS "Build zlib as a static library." ON)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)  # 1
check_include_file(stdint.h HAVE_STDINT_H)  # 1
check_include_file(stddef.h HAVE_STDDEF_H)  # 1

# Check to see if we have large file support
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)

# We add these other definitions here because CheckTypeSize.cmake in CMake 2.4.x donnot automatically do compatibility with CMake 2.4.x
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/crc32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/deflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gzguts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inffast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inffixed.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inftrees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zutil.h
)
set(ZLIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zutil.c
)

if(ZLIB_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(ZLIB_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/win32/zlib1.rc)
    endif()
    # Parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/zlib.h _zlib_h_contents)
    string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*" "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})
    # message(STATUS "zlib_full_version: ${ZLIB_FULL_VERSION}")  # 1.3.1
    if(MINGW)
        # This gets us DLL resource information when compiling on MinGW.
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres.exe)
        endif()
        add_custom_command(OUTPUT ${ZLIB_LIBRARY_OUTPUT_PATH}/zlib1rc.obj 
                COMMAND ${CMAKE_RC_COMPILER}
                -D GCC_WINDRES
                -I ${CMAKE_CURRENT_SOURCE_DIR}/
                -I ${ZLIB_LIBRARY_OUTPUT_PATH}
                -o ${ZLIB_LIBRARY_OUTPUT_PATH}/zlib1rc.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/win32/zlib1.rc
        )
        set(ZLIB_DLL_SRCS ${ZLIB_LIBRARY_OUTPUT_PATH}/zlib1rc.obj)
    endif(MINGW)
endif()

if(ZLIB_BUILD_SHARED_LIBS)
    add_library(zlib SHARED ${ZLIB_SRCS} ${ZLIB_DLL_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
    target_include_directories(zlib PUBLIC ${ZLIB_LIBRARY_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/)
    set_target_properties(zlib PROPERTIES DEFINE_SYMBOL ZLIB_DLL)
    set_target_properties(zlib PROPERTIES SOVERSION 1)
    if(NOT CYGWIN)  # togo
        # This property causes shared libraries on Linux/MINGW to have the full version encoded into their final filename.
        # We disable this on Cygwin because it causes cygz-${ZLIB_FULL_VERSION}.dll to be created when cygz.dll seems to be the default.
        # This has no effect with MSVC, on that platform the version info for the DLL comes from the resource file win32/zlib1.rc
        set_target_properties(zlib PROPERTIES VERSION ${ZLIB_FULL_VERSION})
    endif()
endif()

if(ZLIB_BUILD_STATIC_LIBS)
    add_library(zlibstatic STATIC ${ZLIB_SRCS} ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
    target_include_directories(zlibstatic PUBLIC ${ZLIB_LIBRARY_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()

if(UNIX)  # UNIX/Linux/Darwin
    # On unix-like platforms the shared library is almost always called `libz`
    if(ZLIB_BUILD_SHARED_LIBS)
        set_target_properties(zlib PROPERTIES OUTPUT_NAME z)
        if(NOT APPLE AND NOT(CMAKE_SYSTEM_NAME STREQUAL AIX))
            set_target_properties(zlib PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${CMAKE_CURRENT_SOURCE_DIR}//zlib.map\"")
        endif()
    endif()
    if(ZLIB_BUILD_STATIC_LIBS)
        set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME z)
    endif()
elseif(ZLIB_BUILD_SHARED_LIBS AND WIN32)
    # Create zlib1.dll when building shared library version
    set_target_properties(zlib PROPERTIES SUFFIX "1.dll")
endif()

# if(ZLIB_BUILD_STATIC_LIBS)
#     update_cached_list(PA_LINK_LIBRARIES zlibstatic)
# endif()