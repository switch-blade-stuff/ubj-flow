
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/detail/define.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/ubjf.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/error.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/type.h
        ${CMAKE_CURRENT_LIST_DIR}/detail/token.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf_read.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf_write.h)