
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/define.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf_error.h
        ${CMAKE_CURRENT_LIST_DIR}/type.h
        ${CMAKE_CURRENT_LIST_DIR}/token.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf_read.h
        ${CMAKE_CURRENT_LIST_DIR}/ubjf_write.h)