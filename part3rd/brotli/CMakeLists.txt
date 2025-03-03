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
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_VERSION_MAJOR" BROTLI_VERSION_MAJOR)
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_VERSION_MINOR" BROTLI_VERSION_MINOR)
read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_VERSION_PATCH" BROTLI_VERSION_PATCH)
set(BROTLI_VERSION "${BROTLI_VERSION_MAJOR}.${BROTLI_VERSION_MINOR}.${BROTLI_VERSION_PATCH}")
# message(STATUS "brotli version: ${BROTLI_VERSION}")  # 1.1.0

# ABI Version information
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_ABI_CURRENT" BROTLI_ABI_CURRENT)
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_ABI_REVISION" BROTLI_ABI_REVISION)
# read_macro("${CMAKE_CURRENT_SOURCE_DIR}/c/common/version.h" "BROTLI_ABI_AGE" BROTLI_ABI_AGE)
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
    "${CMAKE_CURRENT_SOURCE_DIR}/c/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/c/common"
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
file(GLOB BROTLI_COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/c/common/*.c)
file(GLOB BROTLI_DEC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/c/dec/*.c)
file(GLOB BROTLI_ENC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/c/enc/*.c)

if(BROTLI_BUILD_SHARED_LIBS)
    if(NOT MINGW)
        set(BROTLI_DLL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/brotli.rc)
    endif()
    if(MINGW)
        # This gets us DLL resource information when compiling on MinGW.
        if(NOT CMAKE_RC_COMPILER)
            set(CMAKE_RC_COMPILER windres.exe)
        endif()
        add_custom_command(OUTPUT ${BROTLI_LIBRARY_OUTPUT_PATH}/brotli.obj 
                COMMAND ${CMAKE_RC_COMPILER}
                -D GCC_WINDRES
                -I ${CMAKE_CURRENT_SOURCE_DIR}/c/include
                -I ${BROTLI_LIBRARY_OUTPUT_PATH}
                -o ${BROTLI_LIBRARY_OUTPUT_PATH}/brotli.obj 
                -i ${CMAKE_CURRENT_SOURCE_DIR}/brotli.rc
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
