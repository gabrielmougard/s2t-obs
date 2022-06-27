cmake_minimum_required(VERSION 3.20)

project(s2t-obs)

set(CMAKE_CXX_STANDARD 14)
set(VERSION_STRING "0.1")

add_compile_definitions(VERSION_STRING="${VERSION_STRING}")

set(THIRD_PARTY_DIR "third_party")
set(CONCURRENTQUEUE_DIR "${THIRD_PARTY_DIR}/concurrentqueue")
set(SPDLOG_DIR "${THIRD_PARTY_DIR}/spdlog")
set(QTBASE_DIR "${THIRD_PARTY_DIR}/qtbase")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_subdirectory(${CONCURRENTQUEUE_DIR})
add_subdirectory(${SPDLOG_DIR})
execute_process(
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${QTBASE_DIR}
    COMMAND "./configure" "-release" 
)
add_subdirectory(${QTBASE_DIR})

set(S2T_OBS_PLUGIN_SRCS
    src/plugin.cc
)

# set(S2T_OBS_PLUGIN_HEADERS
#     ${CONCURRENTQUEUE_DIR}/concurrentqueue.h
#     ${CONCURRENTQUEUE_DIR}/blockingconcurrentqueue.h
# )

add_library(s2t-obs-plugin MODULE
    ${S2T_OBS_PLUGIN_SRCS}
)

target_link_libraries(s2t-obs-plugin
    concurrentqueue
    spdlog::spdlog
    Qt6::Widgets
)
