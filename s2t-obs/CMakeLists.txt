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

# Automatic call to UIC compiler to generate header files for .ui descriptions.
qt_standard_project_setup()
set(obs_QRC src/resources/obs.qrc)
qt6_add_resources(obs_QRC_SOURCES ${obs_QRC})

# Protos generation
set(PROTOS protos/example.proto)
set(PROTO_GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_GENERATED_PATH})
set(GRPC_GENERATED_PATH ${PROTO_GENERATED_PATH}) # consider adding separate folder for grpc generated files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_GENERATED_PATH} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${GRPC_GENERATED_PATH} ${PROTOS})

set(S2T_OBS_PLUGIN_HEADERS
    ${PROTOBUF_INCLUDE_DIRS}
    ${GRPC_GENERATED_PATH}
)

set(S2T_OBS_PLUGIN_SRCS
    ${PROTO_SRCS}
    ${GRPC_SRCS}
    src/plugin.cc
)

add_library(s2t-obs-plugin MODULE
    ${S2T_OBS_PLUGIN_HEADERS}
    ${S2T_OBS_PLUGIN_SRCS}
    ${obs_QRC_SOURCES}
)

target_link_libraries(s2t-obs-plugin
    concurrentqueue::concurrentqueue
    spdlog::spdlog
    Qt6::Widgets
    OBS::libobs
    OBS::obs-frontend-api
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

install(TARGETS s2t-obs-plugin DESTINATION lib)