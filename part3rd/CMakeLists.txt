# ------------------------------------------------------------------------
# zlib:
# ------------------------------------------------------------------------
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
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/crc32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/deflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/gzguts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inffast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inffixed.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inftrees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/trees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zutil.h
)
set(ZLIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zutil.c
)

if(ZLIB_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(ZLIB_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/win32/zlib1.rc)
    endif()
    # Parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zlib.h _zlib_h_contents)
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
                -I ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1
                -I ${ZLIB_LIBRARY_OUTPUT_PATH}
                -o ${ZLIB_LIBRARY_OUTPUT_PATH}/zlib1rc.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/win32/zlib1.rc
        )
        set(ZLIB_DLL_SRCS ${ZLIB_LIBRARY_OUTPUT_PATH}/zlib1rc.obj)
    endif(MINGW)
endif()

if(ZLIB_BUILD_SHARED_LIBS)
    add_library(zlib SHARED ${ZLIB_SRCS} ${ZLIB_DLL_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
    target_include_directories(zlib PUBLIC ${ZLIB_LIBRARY_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1)
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
    target_include_directories(zlibstatic PUBLIC ${ZLIB_LIBRARY_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1)
endif()

if(UNIX)  # UNIX/Linux/Darwin
    # On unix-like platforms the shared library is almost always called `libz`
    if(ZLIB_BUILD_SHARED_LIBS)
        set_target_properties(zlib PROPERTIES OUTPUT_NAME z)
        if(NOT APPLE AND NOT(CMAKE_SYSTEM_NAME STREQUAL AIX))
            set_target_properties(zlib PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1/zlib.map\"")
        endif()
    endif()
    if(ZLIB_BUILD_STATIC_LIBS)
        set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME z)
    endif()
elseif(ZLIB_BUILD_SHARED_LIBS AND WIN32)
    # Create zlib1.dll when building shared library version
    set_target_properties(zlib PROPERTIES SUFFIX "1.dll")
endif()

if(ZLIB_BUILD_STATIC_LIBS)
    update_cached_list(PA_LINK_LIBRARIES zlibstatic)
endif()
# ------------------------------------------------------------------------
# @end zlib
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# jsoncpp:
# ------------------------------------------------------------------------
set(JSONCPP_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(JSONCPP_BUILD_SHARED_LIBS "Build jsoncpp as a shared library." ON)
option(JSONCPP_BUILD_STATIC_LIBS "Build jsoncpp as a static library." ON)

file(GLOB JSONCPP_SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/include/json/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/src/lib_json/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/src/lib_json/*.h 
)

if(JSONCPP_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(JSONCPP_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/jsoncpp.rc)
    endif()
    # Parse the full version number from version.h and include in JSONCPP_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/include/json/version.h _jsoncpp_version_h_contents)
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
                -I ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6
                -I ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/include
                -I ${JSONCPP_LIBRARY_OUTPUT_PATH}
                -o ${JSONCPP_LIBRARY_OUTPUT_PATH}/jsoncpp.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp-1.9.6/jsoncpp.rc
        )
        set(JSONCPP_DLL_SRCS ${JSONCPP_LIBRARY_OUTPUT_PATH}/jsoncpp.obj)
    endif(MINGW)
endif()
if(JSONCPP_BUILD_SHARED_LIBS)
    add_library(jsoncpp SHARED ${JSONCPP_DLL_SRCS} ${JSONCPP_SRC_LIST})
    target_compile_options(jsoncpp PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(jsoncpp 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/jsoncpp-1.9.6/include"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/jsoncpp-1.9.6/include"
    )
    set_target_properties(jsoncpp PROPERTIES DEFINE_SYMBOL JSONCPP_DLL)
    set_target_properties(jsoncpp PROPERTIES SOVERSION 26)
endif()

if(JSONCPP_BUILD_STATIC_LIBS)
    add_library(jsoncppstatic STATIC ${JSONCPP_SRC_LIST})
    target_compile_options(jsoncppstatic PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(jsoncppstatic 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/jsoncpp-1.9.6/include"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/jsoncpp-1.9.6/include"
    )
    update_cached_list(PA_LINK_LIBRARIES jsoncppstatic)
endif()
# ------------------------------------------------------------------------
# @end jsoncpp
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# sqlite3:
# ------------------------------------------------------------------------
set(SQLITE3_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(SQLITE3_BUILD_SHARED_LIBS "Build sqlite3 as a shared library." ON)
option(SQLITE3_BUILD_STATIC_LIBS "Build sqlite3 as a static library." ON)

file(GLOB SQLITE3_SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/sqlite3.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/sqlite3.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/sqlite3ext.h 
)

if(SQLITE3_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(SQLITE3_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/art/sqlite3.rc)
    endif()
    # Parse the full version number from sqlite3.h and include in SQLITE3_FULL_VERSION
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/sqlite3.h _sqlite3_h_contents)
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
                -I ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000
                -I ${SQLITE3_LIBRARY_OUTPUT_PATH}
                -o ${SQLITE3_LIBRARY_OUTPUT_PATH}/sqlite3.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-autoconf-3480000/art/sqlite3.rc
        )
        set(SQLITE3_DLL_SRCS ${SQLITE3_LIBRARY_OUTPUT_PATH}/sqlite3.obj)
    endif(MINGW)
endif()

if(SQLITE3_BUILD_SHARED_LIBS)
    add_library(sqlite3 SHARED ${SQLITE3_DLL_SRCS} ${SQLITE3_SRC_LIST})
    target_compile_options(sqlite3 PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_include_directories(sqlite3 
        PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/sqlite-autoconf-3480000"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/sqlite-autoconf-3480000"
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
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/sqlite-autoconf-3480000"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/sqlite-autoconf-3480000"
    )
    update_cached_list(PA_LINK_LIBRARIES sqlite3static)
endif()

# ------------------------------------------------------------------------
# @end sqlite3
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# brotli:
# ------------------------------------------------------------------------
set(BROTLI_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
option(BROTLI_BUILD_SHARED_LIBS "Build brotli as a shared library." ON)
option(BROTLI_BUILD_STATIC_LIBS "Build brotli as a static library." ON)

# Reads macro from .h file, it is expected to be a single-line define.
function(read_macro PATH MACRO OUTPUT)
    file(STRINGS ${PATH} _line REGEX "^#define +${MACRO} +(.+)$")
    string(REGEX REPLACE "^#define +${MACRO} +(.+)$" "\\1" _val "${_line}")
    set(${OUTPUT} ${_val} PARENT_SCOPE)
endfunction(read_macro)

# version information
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_VERSION_MAJOR" BROTLI_VERSION_MAJOR)
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_VERSION_MINOR" BROTLI_VERSION_MINOR)
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_VERSION_PATCH" BROTLI_VERSION_PATCH)
set(BROTLI_VERSION "${BROTLI_VERSION_MAJOR}.${BROTLI_VERSION_MINOR}.${BROTLI_VERSION_PATCH}")
# message(STATUS "brotli version: ${BROTLI_VERSION}")  # 1.1.0

# ABI Version information
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_ABI_CURRENT" BROTLI_ABI_CURRENT)
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_ABI_REVISION" BROTLI_ABI_REVISION)
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/version.h" "BROTLI_ABI_AGE" BROTLI_ABI_AGE)
# math(EXPR BROTLI_ABI_COMPATIBILITY "${BROTLI_ABI_CURRENT} - ${BROTLI_ABI_AGE}")
# message(STATUS "BROTLI_API_COMPATI: ${BROTLI_ABI_COMPATIBILITY}")  # 1

include(CheckFunctionExists)
set(LIBM_LIBRARY)
CHECK_FUNCTION_EXISTS(log2 LOG2_RES)   # log2,LOG2_RES - found
if(NOT LOG2_RES)
  set(orig_req_libs "${CMAKE_REQUIRED_LIBRARIES}")
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
  CHECK_FUNCTION_EXISTS(log2 LOG2_LIBM_RES)
  if(LOG2_LIBM_RES)
    set(LIBM_LIBRARY "m")
    add_definitions(-DBROTLI_HAVE_LOG2=1)
  else()
    add_definitions(-DBROTLI_HAVE_LOG2=0)
  endif()
  set(CMAKE_REQUIRED_LIBRARIES "${orig_req_libs}")
  unset(LOG2_LIBM_RES)
  unset(orig_req_libs)
else()
  add_definitions(-DBROTLI_HAVE_LOG2=1)
endif()
unset(LOG2_RES)
# message(STATUS "CMAKE_REQUIRED_LIBRARIES: ${CMAKE_REQUIRED_LIBRARIES}")  # nothing

set(BROTLI_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common"
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DOS_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  add_definitions(-DOS_FREEBSD)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DOS_MACOSX)
  set(CMAKE_MACOS_RPATH TRUE)
endif()

# select source codes
file(GLOB BROTLI_COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/common/*.c)
file(GLOB BROTLI_DEC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/dec/*.c)
file(GLOB BROTLI_ENC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/enc/*.c)

if(BROTLI_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(BROTLI_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/brotli.rc)
    endif()
    if(MINGW)
        # This gets us DLL resource information when compiling on MinGW.
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres.exe)
        endif()
        add_custom_command(OUTPUT ${BROTLI_LIBRARY_OUTPUT_PATH}/brotli.obj 
                COMMAND ${CMAKE_RC_COMPILER}
                -D GCC_WINDRES
                -I ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/c/include
                -I ${BROTLI_LIBRARY_OUTPUT_PATH}
                -o ${BROTLI_LIBRARY_OUTPUT_PATH}/brotli.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/brotli-1.1.0/brotli.rc
        )
        set(BROTLI_DLL_SRCS ${BROTLI_LIBRARY_OUTPUT_PATH}/brotli.obj)
    endif(MINGW)
endif()
if(BROTLI_BUILD_SHARED_LIBS)
    set(BROTLI_LIBRARIES_CORE brotlienc brotlidec brotlicommon)
    set(BROTLI_LIBRARIES ${BROTLI_LIBRARIES_CORE} ${LIBM_LIBRARY})
    # message(STATUS "brotli_libraries: ${BROTLI_LIBRARIES}")  # brotlienc;brotlidec;brotlicommon
    add_library(brotlicommon SHARED ${BROTLI_DLL_SRCS} ${BROTLI_COMMON_SOURCES})
    add_library(brotlidec SHARED ${BROTLI_DLL_SRCS} ${BROTLI_DEC_SOURCES})
    target_link_libraries(brotlidec brotlicommon ${LIBM_LIBRARY})  # dec依赖comon
    add_library(brotlienc SHARED ${BROTLI_DLL_SRCS} ${BROTLI_ENC_SOURCES})
    target_link_libraries(brotlienc brotlicommon ${LIBM_LIBRARY})  # enc依赖comon
    include_directories(${BROTLI_INCLUDE_DIRS})
    foreach(lib ${BROTLI_LIBRARIES_CORE})
        target_compile_definitions(${lib} PUBLIC "BROTLI_SHARED_COMPILATION")
        string(TOUPPER "${lib}" LIB)
        target_compile_options(${lib} PRIVATE ${COMPILE_OPTIONS_DEFAULT})
        set_target_properties(${lib} PROPERTIES DEFINE_SYMBOL "${LIB}_SHARED_COMPILATION}")
    endforeach(lib)
endif()

if(BROTLI_BUILD_STATIC_LIBS)
    set(BROTLI_LIBRARIES_CORE brotliencstatic brotlidecstatic brotlicommonstatic)
    set(BROTLI_LIBRARIES ${BROTLI_LIBRARIES_CORE} ${LIBM_LIBRARY})
    add_library(brotlicommonstatic STATIC ${BROTLI_COMMON_SOURCES})
    add_library(brotlidecstatic STATIC ${BROTLI_DEC_SOURCES})
    add_library(brotliencstatic STATIC ${BROTLI_ENC_SOURCES})
    include_directories(${BROTLI_INCLUDE_DIRS})
    foreach(lib ${BROTLI_LIBRARIES_CORE})
        target_compile_definitions(${lib} PUBLIC ${COMPILE_DEFINITIONS})
        target_compile_options(${lib} PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    endforeach(lib)
endif() 

foreach(lib ${BROTLI_LIBRARIES_CORE})
  target_link_libraries(${lib} ${LIBM_LIBRARY})
  set_property(TARGET ${lib} APPEND PROPERTY INCLUDE_DIRECTORIES ${BROTLI_INCLUDE_DIRS})
#   set_target_properties(${lib} PROPERTIES
#     VERSION "${BROTLI_ABI_COMPATIBILITY}.${BROTLI_ABI_AGE}.${BROTLI_ABI_REVISION}"
#     SOVERSION "${BROTLI_ABI_COMPATIBILITY}")
  set_property(TARGET ${lib} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${BROTLI_INCLUDE_DIRS}>")
endforeach()

if(BROTLI_BUILD_STATIC_LIBS)
    update_cached_list(PA_LINK_LIBRARIES ${BROTLI_LIBRARIES_CORE})
endif()

# ------------------------------------------------------------------------
# @end brotli
# ------------------------------------------------------------------------

# ------------------------------------------------------------------------
# toolkit:
# ------------------------------------------------------------------------
# TODO: 改造toolkit以便直接引用

include(CheckStructHasMember)
include(CheckSymbolExists)

# 检查 sendmmsg 相关依赖并设置对应的宏，配置 _GNU_SOURCE 以启用 GNU 扩展特性
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_struct_has_member("struct mmsghdr" msg_hdr sys/socket.h HAVE_MMSG_HDR)  # FALSE
check_symbol_exists(sendmmsg sys/socket.h HAVE_SENDMMSG_API)
check_symbol_exists(recvmmsg sys/socket.h HAVE_RECVMMSG_API)

set(COMPILE_DEFINITIONS)
# ToolKit 依赖 ENABLE_OPENSSL
# list(FIND PA_COMPILE_DEFINITIONS ENABLE_OPENSSL ENABLE_OPENSSL_INDEX)
# message(STATUS "openssl index: ${ENABLE_OPENSSL_INDEX}")  # -1 因为我们没有加入到 PA_COMPILE_DEFINITIONS 中！
# if(NOT ENABLE_OPENSSL_INDEX EQUAL -1)
#   list(APPEND COMPILE_DEFINITIONS ENABLE_OPENSSL)
# endif()
if(HAVE_MMSG_HDR)
  list(APPEND COMPILE_DEFINITIONS HAVE_MMSG_HDR)
endif()
if(HAVE_SENDMMSG_API)
  list(APPEND COMPILE_DEFINITIONS HAVE_SENDMMSG_API)
endif()
if(HAVE_RECVMMSG_API)
  list(APPEND COMPILE_DEFINITIONS HAVE_RECVMMSG_API)
endif()

# Check the socket buffer size set by the upper cmake project, if it is set, use the setting of the upper cmake project, otherwise set it to 256K
# If the socket buffer size is set to 0, it means that the socket buffer size is not set, and the kernel default value is used(just for linux)
if(DEFINED SOCKET_DEFAULT_BUF_SIZE)
  if(SOCKET_DEFAULT_BUF_SIZE EQUAL 0)
    message(STATUS "Socket default buffer size is not set, use the kernel default value")
  else()
    message(STATUS "Socket default buffer size is set to ${SOCKET_DEFAULT_BUF_SIZE}")
  endif()
  add_definitions(-DSOCKET_DEFAULT_BUF_SIZE=${SOCKET_DEFAULT_BUF_SIZE})
endif()

set(TOOLKIT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/toolkit)
file(GLOB TOOLKIT_SRC_LIST 
    ${TOOLKIT_ROOT}/src/*/*.cpp 
    ${TOOLKIT_ROOT}/src/*/*.h 
    ${TOOLKIT_ROOT}/src/*/*.c 
)
if(IOS)
    list(APPEND TOOLKIT_SRC_LIST 
        ${TOOLKIT_ROOT}/src/Network/Socket_ios.mm)
endif()

# 使用 wepoll windows iocp 模拟 linux's epoll
if(ENABLE_WEPOLL)
    if(WIN32)
        message(STATUS "Enable wepoll")
        # 增加 wepoll 源文件及 api 参数兼容文件
        list(APPEND TOOLKIT_SRC_LIST 
                ${TOOLKIT_ROOT}/wepoll/wepoll.c 
                ${TOOLKIT_ROOT}/wepoll/wepoll.h  
                ${TOOLKIT_ROOT}/wepoll/sys/epoll.cpp  
                ${TOOLKIT_ROOT}/wepoll/sys/epoll.h   
        )
        message(STATUS, "toolkit: ${TOOLKIT_SRC_LIST}")
        # 增加 wepoll 头文件目录
        include_directories(${TOOLKIT_ROOT}/wepoll ${TOOLKIT_ROOT}/wepoll/sys)
        # 开启epoll
        add_definitions(-DHAS_EPOLL)
    endif()
endif()

# 去除 win32 的适配代码
if(NOT WIN32)
  list(REMOVE_ITEM TOOLKIT_SRC_LIST ${TOOLKIT_ROOT}/win32/getopt.c)
else()
  # 防止 Windows.h 包含 Winsock.h
  list(APPEND COMPILE_DEFINITIONS
    WIN32_LEAN_AND_MEAN MP4V2_NO_STDINT_DEFS
    # 禁用警告
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_library(toolkit STATIC ${TOOLKIT_SRC_LIST})
target_compile_definitions(toolkit PUBLIC ${COMPILE_DEFINITIONS})
target_compile_options(toolkit PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_include_directories(toolkit
  PRIVATE
    "$<BUILD_INTERFACE:${TOOLKIT_ROOT}/src>" "$<BUILD_INTERFACE:${TOOLKIT_ROOT}/wepoll>" "$<BUILD_INTERFACE:${TOOLKIT_ROOT}/wepoll/sys>"
  PUBLIC
    "$<BUILD_INTERFACE:${TOOLKIT_ROOT}>/src" "$<BUILD_INTERFACE:${TOOLKIT_ROOT}/wepoll>" "$<BUILD_INTERFACE:${TOOLKIT_ROOT}/wepoll/sys>"
)

update_cached_list(PA_LINK_LIBRARIES toolkit)
# ------------------------------------------------------------------------
# @end toolkit
# ------------------------------------------------------------------------
