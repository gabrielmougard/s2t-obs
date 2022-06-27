cmake_minimum_required(VERSION 3.20)
project(grpc)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(UNIX)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(PROTOBUF_TARGET protobuf)
  set(GRPC_TARGET grpc)
  set(GRPC_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/grpc)

  set(PROTOBUF_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROTOBUF_TARGET})
  link_directories(${GRPC_LIB_DIR} ${PROTOBUF_LIB_DIR} ${CARES_LIB_DIR})

  set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "")
  if(OPENSSL_ROOT_DIR)
    set(_CMAKE_ARGS_OPENSSL_ROOT_DIR
        "-DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}")
  endif()

  get_property(HAS_LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

  ExternalProject_Add(
    ${GRPC_TARGET}
    PREFIX grpc
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/grpc"
    CMAKE_CACHE_ARGS -DBUILD_SHARED_LIBS:BOOL=OFF
                     -DgRPC_INSTALL:BOOL=ON
                     -DgRPC_BUILD_TESTS:BOOL=OFF
                     -DgRPC_SSL_PROVIDER:STRING=package
                     ${_CMAKE_ARGS_OPENSSL_ROOT_DIR}
                     -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                     -DCMAKE_INSTALL_PREFIX:PATH=${GRPC_INSTALL_DIR}
                     -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
                     -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
                     -DCMAKE_C_COMPILER_ARG1:STRING=${CMAKE_C_COMPILER_ARG1}
                     -DCMAKE_CXX_COMPILER_ARG1:STRING=${CMAKE_CXX_COMPILER_ARG1}
                     -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
                     -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
                     -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
                     -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
                     -DCMAKE_STATIC_LINKER_FLAGS:STRING=${CMAKE_STATIC_LINKER_FLAGS}
                     -DCMAKE_MACOSX_RPATH:BOOL=ON)
  install(DIRECTORY ${GRPC_INSTALL_DIR}/include
          COMPONENT grpc_header
          DESTINATION ${CMAKE_INSTALL_PREFIX}
          USE_SOURCE_PERMISSIONS)

  install(DIRECTORY ${GRPC_INSTALL_DIR}/lib
          COMPONENT libgrpc
          DESTINATION ${CMAKE_INSTALL_PREFIX}
          USE_SOURCE_PERMISSIONS)

  # libprotobuf* are installed under lib64 but libgrpc* are installed under lib
  if(HAS_LIB64 AND NOT APPLE)
    install(FILES ${GRPC_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libprotobuf.a
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    install(
      FILES ${GRPC_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libprotobuf.a
      DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    install(FILES ${GRPC_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libprotoc.a
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    install(FILES ${GRPC_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libprotoc.a
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

    install(FILES ${GRPC_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libcares.a
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
  endif()

  install(DIRECTORY ${GRPC_INSTALL_DIR}/bin
          COMPONENT grpc_cpp_plugin
          DESTINATION ${CMAKE_INSTALL_PREFIX}
          USE_SOURCE_PERMISSIONS)
endif()

configure_package_config_file(cmake/grpc-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/grpc-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/grpc-config.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/)

