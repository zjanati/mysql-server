

MYSQL_ADD_EXECUTABLE(fuzzingNetwork fuzzingNetwork.cc)
TARGET_LINK_LIBRARIES(fuzzingNetwork library_mysys)
TARGET_LINK_LIBRARIES(fuzzingNetwork mysqlclient)
TARGET_LINK_LIBRARIES(fuzzingNetwork FuzzingEngine)

SET(LIBFUZZER_LIBRARIES Fuzzer)

MYSQL_ADD_EXECUTABLE(fuzz_router_uri fuzz_router_uri.cc
                        ${PROJECT_SOURCE_DIR}/src/router/src/uri.cc
                          ${PROJECT_SOURCE_DIR}/src/router/src/utils.cc)

TARGET_LINK_LIBRARIES(fuzz_router_uri harness-library ${MySQL_CLIENT_LIB}
                        ${SSL_LIBRARIES} ${LIBFUZZER_LIBRARIES})
  TARGET_INCLUDE_DIRECTORIES(fuzz_router_uri
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/router/include/)
  fuzz(fuzz_router_uri)

add_executable(fuzz_router_uri_tostring
    fuzz_router_uri_tostring.cc
    ${PROJECT_SOURCE_DIR}/src/router/src/uri.cc
    ${PROJECT_SOURCE_DIR}/src/router/src/utils.cc
  )

TARGET_LINK_LIBRARIES(fuzz_router_uri_tostring harness-library ${MySQL_CLIENT_LIB}
                        ${SSL_LIBRARIES} ${LIBFUZZER_LIBRARIES})
  TARGET_INCLUDE_DIRECTORIES(fuzz_router_uri_tostring
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/router/include/)
  fuzz(fuzz_router_uri_tostring)