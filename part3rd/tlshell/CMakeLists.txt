set(TLSHELL_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
# option(TLSHELL_BUILD_SHARED_LIBS "Build tlshell as a shared library." ON)
option(TLSHELL_BUILD_STATIC_LIBS "Build tlshell as a static library." ON)

file(GLOB TLSHELL_PUBLIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/tlshell/inc/*.h
)
file(GLOB TLSHELL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tlshell/src/*.cpp
)

if(TLSHELL_BUILD_STATIC_LIBS)
    add_library(tlshell STATIC ${TLSHELL_SRCS} ${TLSHELL_PUBLIC_HDRS})
    target_include_directories(tlshell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tlshell/inc)
    target_link_libraries(tlshell PRIVATE OpenSSL::SSL OpenSSL::Crypto)

    # find_package(Threads)
    # target_link_libraries(tlshell PUBLIC Threads::Threads)
    # if(WIN32)
    #     target_link_libraries(tlshell PRIVATE ws2_32 rpcrt4)
    # if(OpenSSL_FOUND)
    #     target_link_libraries(tlshell PRIVATE crypt32 secur32)
    # endif(OpenSSL_FOUND)
    # elseif(NOT ANDROID)
    #     target_link_libraries(tlshell PRIVATE pthread  $<$<PLATFORM_ID:SunOS>:socket>)
    # endif(WIN32)
endif()