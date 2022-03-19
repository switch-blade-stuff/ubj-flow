
generate_export_header(${PROJECT_NAME}
        BASE_NAME ubjf
        EXPORT_MACRO_NAME UBJF_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_LIST_DIR}/detail/export.h)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/detail/define.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/error.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/type.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/token.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf.h
        ${CMAKE_CURRENT_LIST_DIR}/read.h
        ${CMAKE_CURRENT_LIST_DIR}/write.h)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/detail/error.c
        ${CMAKE_CURRENT_LIST_DIR}/detail/read_init.c
        ${CMAKE_CURRENT_LIST_DIR}/detail/write_init.c
        ${CMAKE_CURRENT_LIST_DIR}/detail/parser.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/parser.c
        ${CMAKE_CURRENT_LIST_DIR}/detail/emitter.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/emitter.c)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/detail/spec12/parser_spec12.c
        ${CMAKE_CURRENT_LIST_DIR}/detail/spec12/emitter_spec12.c)