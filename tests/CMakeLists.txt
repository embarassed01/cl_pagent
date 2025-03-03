aux_source_directory(. TEST_SRC_LIST)

foreach(TEST_SRC ${TEST_SRC_LIST})
    get_filename_component(TEST_EXE_NAME ${TEST_SRC} NAME_WE)
    message(STATUS "add test: ${TEST_EXE_NAME}")

    add_executable(${TEST_EXE_NAME} ${TEST_SRC})

    target_compile_options(${TEST_EXE_NAME} PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_compile_definitions(${TEST_EXE_NAME} PRIVATE ${PA_COMPILE_DEFINITIONS})
    # if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    #     target_link_libraries(${TEST_EXE_NAME} -Wl,--start-group ${PA_LINK_LIBRARIES} -Wl,--end-group)
    # else()
    target_link_libraries(${TEST_EXE_NAME} -Wl,--start-group ${PA_LINK_LIBRARIES} -Wl,--end-group)

    if("${TEST_EXE_NAME}" STREQUAL "test_zlib")
        set(INCLUDE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/zlib/"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} zlibstatic)
    endif()
    if("${TEST_EXE_NAME}" STREQUAL "test_brotli")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/brotli/c/common"
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/brotli/c/include"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} brotliencstatic brotlidecstatic brotlicommonstatic)
    endif()
    if ("${TEST_EXE_NAME}" STREQUAL "test_jsoncpp")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/jsoncpp/include"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} jsoncppstatic)
    endif()
    if ("${TEST_EXE_NAME}" STREQUAL "test_sqlite")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/sqlite"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} sqlite3static)
    endif()
    if (("${TEST_EXE_NAME}" STREQUAL "test_TcpServer") OR ("${TEST_EXE_NAME}" STREQUAL "test_TcpClient"))
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/trantor"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} trantor)
    endif()
endforeach(TEST_SRC)
