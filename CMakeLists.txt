cmake_minimum_required(VERSION 3.10)

set(CMAKE_STRIP "strip")

project(pagent LANGUAGES CXX C)
# set(CMAKE_C_COMPILER gcc)            # set(CMAKE_CXX_COMPILER g++)
# set(C_STANDARD 20)                   # set(CXX_STANDARD 20)
# set(CMAKE_C_STANDARD 20)             # set(CMAKE_CXX_STANDARD 20)
# set(C_STANDARD_REQUIRED YES)         # set(CXX_STANDARD_REQUIRED YES)
# set(CMAKE_C_STANDARD_REQUIRED YES)   # set(CMAKE_CXX_STANDARD_REQUIRED YES)

option(ENABLE_OPENSSL "Enable OpenSSL" ON)
# option(ENABLE_WEPOLL "Enable wepoll" ON)
# option(ENABLE_MEM_DEBUG "Enable Memory Debug" OFF)
# option(USE_SOLUTION_FOLDERS "Enable solution dir supported" OFF)

# 设置socket默认缓冲区为256K，如果设置为0则不设置socket的默认缓冲区大小，使用系统内核默认值（设置为0仅对Linux有效）
set(SOCKET_DEFAULT_BUF_SIZE 262144 CACHE STRING "Default buffer size for socket" FORCE)

# set(CMAKE_BUILD_TYPE "Release") 
set(CMAKE_BUILD_TYPE "Debug")  # "Release"
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "编译类型：${CMAKE_BUILD_TYPE}")

# 为方便排查编译问题，需要 FORCE CACHE，否则需要命令行设置才能生效
set(CMAKE_VERBOSE_MAKEFILE ON CACHE INTERNAL "" FORCE)

# TODO: include 当前目录可能会编译出错，先取消
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)

# 设置输出目录，包括bin,lib,以及其他文件
set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/${SYSTEM_NAME_LOWER}/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})

# 为方便修改全局变量
function(update_cached name value)
    set("${name}" "${value}" CACHE INTERNAL "*** Internal ***" FORCE)
endfunction()
function(update_cached_list name)
    set(_tmp_list "${${name}}")
    message(STATUS "0.${name} update_cached: ${${name}}")
    list(APPEND _tmp_list "${ARGN}")
    list(REMOVE_DUPLICATES _tmp_list)
    update_cached(${name} "${_tmp_list}")
    message(STATUS "1.${name} update_cached: ${${name}}")
endfunction()

# CMake在各种编译模式下使用的默认选项:
#     Release模式:​​-O3 -DNDEBUG​​
#     Debug模式:​​-g​​
#     MinSizeRel模式:​​-Os -DNDEBUG​​
#     RelWithDebInfo模式:​-O2 -g -DNDEBUG
# 设置默认编译选项
if(UNIX)  # UNIX/Linux/Darwin
    message(STATUS "设置默认编译选项: UNIX/Linux/Darwin")
    set(COMPILE_OPTIONS_DEFAULT 
        "-fPIC"
        "-Wall;-Wextra"
        "-Wno-unused-function;-Wno-unused-parameter;-Wno-unused-variable;-Wno-deprecated-declarations"
        "-Wno-error=extra;-Wno-error=missing-field-initializers;-Wno-error=type-limits;-Wno-comment"
    )
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(COMPILE_OPTIONS_DEFAULT ${COMPILE_OPTIONS_DEFAULT} "-g3")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} -g -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wshadow")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} -g -Wall -Wextra -Wfatal-errors -Wno-unused-parameter")
    else()
        set(COMPILE_OPTIONS_DEFAULT ${COMPILE_OPTIONS_DEFAULT} "-g0")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -DNDEBUG -fvisibility=hidden -ffunction-sections -fdata-sections -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fvtable-gc -fno-stack-protector -fno-rtti -fno-unwind-tables -fno-ident -W -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wshadow")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -DNDEBUG -fvisibility=hidden -ffunction-sections -fdata-sections -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fvtable-gc -fno-stack-protector -fno-rtti -fno-unwind-tables -fno-ident -W -Wall -Wextra -Wfatal-errors -Wno-unused-parameter")
    endif()
elseif(WIN32)
    message(STATUS "设置默认编译选项: Windows")
    if(MSVC)  # not go
        message(STATUS "MSVC")
        set(COMPILE_OPTIONS_DEFAULT 
            # TODO: /wd4819应该不会生效
            "/wd4566;/wd4819;/utf-8"
            # warning C4530: C++ exception handler used, but unwind semantics are not enabled
            "/EHsc"
        )
        # disable Windows logo
        string(REPLACE "/nologo" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_STATUS_LINKER_FLAGS "")
    elseif(MINGW)  # to go
        message(STATUS "MINGW")
        set(COMPILE_OPTIONS_DEFAULT 
            "-fPIC"
            "-Wall;-Wextra;-Wmissing-prototypes;-Wstrict-prototypes"
            "-Wincompatible-pointer-types"
            "-Wno-unused-function;-Wno-unused-parameter;-Wno-unused-variable;-Wno-deprecated-declarations"
            "-Wno-error=extra;-Wno-error=missing-field-initializers;-Wno-error=type-limits;-Wno-comment"
        )
        if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            set(COMPILE_OPTIONS_DEFAULT ${COMPILE_OPTIONS_DEFAULT} "-g3") 
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wshadow")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -Wfatal-errors -Wno-unused-parameter")
        else()
            set(COMPILE_OPTIONS_DEFAULT ${COMPILE_OPTIONS_DEFAULT} "-g0") 
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -DNDEBUG -fvisibility=hidden -ffunction-sections -fdata-sections -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fvtable-gc -fno-stack-protector -fno-rtti -fno-unwind-tables -fno-ident -W -Wall -Wextra -Wfatal-errors -Wno-unused-parameter -Wshadow")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -DNDEBUG -fvisibility=hidden -ffunction-sections -fdata-sections -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fvtable-gc -fno-stack-protector -fno-rtti -fno-unwind-tables -fno-ident -W -Wall -Wextra -Wfatal-errors -Wno-unused-parameter")
        endif()
    endif()
endif()

# MacOS特有
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        include_directories(SYSTEM "/opt/homebrew/include")
    endif()
endif()

# pagent以及各个runtime依赖，仅截止到openssl就不要再更新了！！
update_cached(PA_LINK_LIBRARIES "")
update_cached(PA_COMPILE_DEFINITIONS "")

# if(ENABLE_MEM_DEBUG)
#     message(STATUS "启用内存调试标识")
#     update_cached_list(PA_LINK_LIBRARIES, "-Wl,-wrap,free;-Wl,-wrap,malloc;-Wl,-wrap,realloc;-Wl,-wrap,calloc")
#     update_cached_list(PA_COMPILE_DEFINITIONS ENABLE_MEM_DEBUG)
# endif()

# static build basic{libgcc, libstdc++, winpthread/pthread} on win32+mingw
# Note: -lwinpthread / -lpthread，只能同时存在一个； -s就是strip symbols
if(MINGW)
    update_cached_list(PA_LINK_LIBRARIES ${COMPILE_OPTIONS_DEFAULT})
    update_cached_list(PA_LINK_LIBRARIES "-s -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libgcc -static-libstdc++")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")  # clang 
    update_cached_list(PA_LINK_LIBRARIES ${COMPILE_OPTIONS_DEFAULT})
    update_cached_list(PA_LINK_LIBRARIES "-s -lpthread -static-libgcc -static-libstdc++")
else()
    update_cached_list(PA_LINK_LIBRARIES ${COMPILE_OPTIONS_DEFAULT})
    update_cached_list(PA_LINK_LIBRARIES "-s -static -static-libgcc -static-libstdc++")  # -static -lpthread -ldl -lm -Wl,-rpath
endif()

find_package(Threads)
update_cached_list(PA_LINK_LIBRARIES Threads::Threads atomic)

if(WIN32)
    update_cached_list(PA_LINK_LIBRARIES ws2_32 rpcrt4 Iphlpapi shlwapi)
elseif(NOT ANDROID OR IOS)  # -Wl,--start-group    -Wl,--end-group
    update_cached_list(PA_LINK_LIBRARIES pthread)
endif()

# some code from `CMake  FindOpenSSL.cmake`
set(OPENSSL_USE_STATIC_LIBS true)
function(_OpenSSL_target_add_dependencies target)
  if(_OpenSSL_has_dependencies)
    if(_OpenSSL_has_dependency_zlib)
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)
    endif()
    if(_OpenSSL_has_dependency_threads)
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES Threads::Threads)
    endif()
    if(_OpenSSL_has_dependency_dl)
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
    endif()
    if(_OpenSSL_extra_static_deps)
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${_OpenSSL_extra_static_deps})
    endif()
  endif()
  if(WIN32 AND OPENSSL_USE_STATIC_LIBS)
    if(WINCE)
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ws2)
    else()
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ws2_32)
    endif()
    set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_LIBRARIES crypt32)
  endif()
endfunction()

# openssl查找是否存在
set(OPENSSL_USE_STATIC_LIBS TRUE)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # find_package(OpenSSL)  # Windows下直接找会有如下找不到的提示
    # Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the filesystem variable OPENSSL_ROOT_DIR (missing: OPENSSL_CRYPTO_LIBRARY OPENSSL_INCLUDE_DIR)
    set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/part3rd/openssl-1.1.1d-win64-mingw)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.a)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.a)
    set(OPENSSL_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
    set(OPENSSL_FOUND true)

    if(NOT TARGET OpenSSL::Crypto AND
        (EXISTS "${OPENSSL_CRYPTO_LIBRARY}" OR
        EXISTS "${LIB_EAY_LIBRARY_DEBUG}" OR
        EXISTS "${LIB_EAY_LIBRARY_RELEASE}")
    )
        add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
        set_target_properties(OpenSSL::Crypto PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
        if(EXISTS "${OPENSSL_CRYPTO_LIBRARY}")
            set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}")
        endif()
        if(EXISTS "${LIB_EAY_LIBRARY_RELEASE}")
            set_property(TARGET OpenSSL::Crypto APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
            IMPORTED_LOCATION_RELEASE "${LIB_EAY_LIBRARY_RELEASE}")
        endif()
        if(EXISTS "${LIB_EAY_LIBRARY_DEBUG}")
            set_property(TARGET OpenSSL::Crypto APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
            IMPORTED_LOCATION_DEBUG "${LIB_EAY_LIBRARY_DEBUG}")
        endif()
        _OpenSSL_target_add_dependencies(OpenSSL::Crypto)
    endif()

    if(NOT TARGET OpenSSL::SSL AND 
        (EXISTS "${OPENSSL_SSL_LIBRARY}" OR
        EXISTS "${SSL_EAY_LIBRARY_DEBUG}" OR
        EXISTS "${SSL_EAY_LIBRARY_RELEASE}")
    )
        add_library(OpenSSL::SSL UNKNOWN IMPORTED)
        set_target_properties(OpenSSL::SSL PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
        if(EXISTS "${OPENSSL_SSL_LIBRARY}")
            set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}")
        endif()
        if(EXISTS "${SSL_EAY_LIBRARY_RELEASE}")
            set_property(TARGET OpenSSL::SSL APPEND PROPERTY
            IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
            IMPORTED_LOCATION_RELEASE "${SSL_EAY_LIBRARY_RELEASE}")
        endif()
        if(EXISTS "${SSL_EAY_LIBRARY_DEBUG}")
            set_property(TARGET OpenSSL::SSL APPEND PROPERTY
            IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
            IMPORTED_LOCATION_DEBUG "${SSL_EAY_LIBRARY_DEBUG}")
        endif()
        if(TARGET OpenSSL::Crypto)
            set_target_properties(OpenSSL::SSL PROPERTIES
            INTERFACE_LINK_LIBRARIES OpenSSL::Crypto)
        endif()
        _OpenSSL_target_add_dependencies(OpenSSL::SSL)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/part3rd/openssl-1.1.1w-linux-gcc)
    find_package(OpenSSL)  # 会自动找到${OPENSSL_INCLUDE_DIR}和${OPENSSL_LIBRARIES},OpenSSL::SSL,OpenSSL:Crypto
else()  # Darwin/Unix
    find_package(OpenSSL)  # 会自动找到${OPENSSL_INCLUDE_DIR}和${OPENSSL_LIBRARIES},OpenSSL::SSL,OpenSSL:Crypto
endif()

if(OPENSSL_FOUND AND ENABLE_OPENSSL)
    message(STATUS "found library: ${OPENSSL_LIBRARIES}, ENABLE_OPENSSL defined")
    include_directories(${OPENSSL_INCLUDE_DIR})
    update_cached_list(PA_LINK_LIBRARIES ${OPENSSL_LIBRARIES})
    if(OPENSSL_USE_STATIC_LIBS AND CMAKE_SYSTEM_NAME MATCHES "Windows")
        update_cached_list(PA_LINK_LIBRARIES Crypt32 secur32)
    elseif(OPENSSL_USE_STATIC_LIBS AND CMAKE_SYSTEM_NAME MATCHES "Linux")
        update_cached_list(PA_LINK_LIBRARIES ${CMAKE_DL_LIBS})  # -ldl
    endif()
else()
    set(ENABLE_OPENSSL OFF)
    message(WARNING "openssl未找到")
endif()

# ------------------------------------------------------------------------
# Solution folders:
# ------------------------------------------------------------------------
# if(USE_SOLUTION_FOLDERS)
#     set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#     set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
# endif()
# if(MSVC AND ENABLE_MSVC_MT)
#     set(CompilerFlags 
#         CMAKE_CXX_FLAGS 
#         CMAKE_CXX_FLAGS_DEBUG 
#         CMAKE_CXX_FLAGS_RELEASE 
#         CMAKE_C_FLAGS 
#         CMAKE_C_FLAGS_DEBUG 
#         CMAKE_C_FLAGS_RELEASE 
#     )
#     # TODO: 通常应该不需要替换
#     foreach(CompilerFlag ${CompilerFlags})
#         string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
#     endforeach()
# endif()
# ------------------------------------------------------------------------
# @end Solution folders
# ------------------------------------------------------------------------

message(STATUS "${PA_LINK_LIBRARIES}")  # the global var: 到此为止，不要再更新！！具体程序根据其使用的库自定义选择！
# -fPIC;-Wall;-Wextra;-Wno-unused-function;-Wno-unused-parameter;-Wno-unused-variable;-Wno-deprecated-declarations;-Wno-error=extra;-Wno-error=missing-field-initializers;-Wno-error=type-limits;-Wno-comment;-g3;-s -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libgcc -static-libstdc++;Threads::Threads;atomic;ws2_32;rpcrt4;Iphlpapi;shlwapi;C:/work/Development/projs/clang/windows/cl_pagent/part3rd/openssl-1.1.1d-win64-mingw/lib/libssl.a;C:/work/Development/projs/clang/windows/cl_pagent/part3rd/openssl-1.1.1d-win64-mingw/lib/libcrypto.a;Crypt32

add_subdirectory(part3rd)
add_subdirectory(tests)

# collect source code
file(GLOB PAGENT_SRC_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*c 
)

add_executable(pagent ${PAGENT_SRC_LIST})


target_compile_options(pagent PRIVATE ${PA_COMPILE_DEFINITIONS})
target_link_libraries(pagent ${PA_LINK_LIBRARIES})

# 为了更大缩减程序体积，可以用upx来压缩。这样压缩的好处是在减少体积的同时不影响程序的执行
# upx --lzma test_TcpServer
