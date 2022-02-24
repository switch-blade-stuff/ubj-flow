project(ubjf.test C)

add_executable(${PROJECT_NAME} main.c)
target_include_directories(${PROJECT_NAME} PRIVATE ../ubjf)
target_link_libraries(${PROJECT_NAME} ubjf)