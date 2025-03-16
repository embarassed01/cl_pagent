set(MPSHELL_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
# option(MPSHELL_BUILD_SHARED_LIBS "Build zlib as a shared library." ON)
option(MPSHELL_BUILD_STATIC_LIBS "Build mpshell as a static library." ON)

file(GLOB MPSHELL_PUBLIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/mpshell/inc/*.h
)
file(GLOB MPSHELL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mpshell/src/*.c
)

if(MPSHELL_BUILD_STATIC_LIBS)
    add_library(mpshell STATIC ${MPSHELL_SRCS} ${MPSHELL_PUBLIC_HDRS})
    target_include_directories(mpshell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mpshell/inc)
    target_link_libraries(mpshell PRIVATE ws2_32 Iphlpapi)
endif()