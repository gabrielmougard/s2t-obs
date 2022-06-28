cmake_minimum_required(VERSION 3.20)

project(s2t-obs)

set(CMAKE_CXX_STANDARD 14)
set(VERSION_STRING "0.1")

add_compile_definitions(VERSION_STRING="${VERSION_STRING}")

# Add path for custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

find_package(MyProtobuf REQUIRED)
find_package(MyGRPC REQUIRED)
find_package(spdlog REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Core REQUIRED)
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

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
    concurrentqueue::concurrentqueue
    spdlog::spdlog
    Qt6::Widgets
    OBS::libobs
    OBS::obs-frontend-api
)

install(TARGETS s2t-obs-plugin DESTINATION lib)