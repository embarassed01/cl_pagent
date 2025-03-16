aux_source_directory(. TEST_SRC_LIST)

foreach(TEST_SRC ${TEST_SRC_LIST})
    get_filename_component(TEST_EXE_NAME ${TEST_SRC} NAME_WE)
    message(STATUS "add test: ${TEST_EXE_NAME}")

    add_executable(${TEST_EXE_NAME} ${TEST_SRC})

    target_compile_options(${TEST_EXE_NAME} PRIVATE ${COMPILE_OPTIONS_DEFAULT})
    target_compile_definitions(${TEST_EXE_NAME} PRIVATE ${PA_COMPILE_DEFINITIONS})
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        target_link_libraries(${TEST_EXE_NAME} ${PA_LINK_LIBRARIES})
    else()  # Linux / WIN32(mingw)
        target_link_libraries(${TEST_EXE_NAME} -Wl,--start-group ${PA_LINK_LIBRARIES} -Wl,--end-group)
    endif()
    

    if("${TEST_EXE_NAME}" STREQUAL "test_zlib")
        set(INCLUDE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/zlib/"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} zlibstatic)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if("${TEST_EXE_NAME}" STREQUAL "test_brotli")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/brotli/c/common"
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/brotli/c/include"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} brotliencstatic brotlidecstatic brotlicommonstatic)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if ("${TEST_EXE_NAME}" STREQUAL "test_jsoncpp")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/jsoncpp/include"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} jsoncppstatic)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if ("${TEST_EXE_NAME}" STREQUAL "test_sqlite")
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/sqlite"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} sqlite3static)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if (("${TEST_EXE_NAME}" STREQUAL "test_TcpServer") OR ("${TEST_EXE_NAME}" STREQUAL "test_TcpClient"))
        set(INCLUDE_DIRS 
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/trantor"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} trantor)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if("${TEST_EXE_NAME}" STREQUAL "test_revshell")
        set(INCLUDE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/revshell/"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} revshell)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if("${TEST_EXE_NAME}" STREQUAL "test_mpshell")
        set(INCLUDE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/mpshell/"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} mpshell)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
    if("${TEST_EXE_NAME}" STREQUAL "test_tlshell_client")
        set(INCLUDE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/../part3rd/tlshell/"
        )
        include_directories(${INCLUDE_DIRS})
        target_link_libraries(${TEST_EXE_NAME} tlshell)

        add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD
                   COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${TEST_EXE_NAME}>"
                   COMMENT "Strip debug symbols done on final binary.")
    endif()
endforeach(TEST_SRC)
