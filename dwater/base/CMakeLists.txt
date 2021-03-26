SET(SRC 
    async_logging.cc
    condition.cc
    count_down_latch.cc
    current_thread.cc
    date.cc
    exception.cc
    file_util.cc
    log_file.cc
    logging.cc
    log_stream.cc
    process_info.cc
    thread.cc
    timestamp.cc
    time_zone.cc)

ADD_LIBRARY(dwater_base ${SRC})

TARGET_LINK_LIBRARIES(dwater_base pthread rt)

INSTALL(TARGETS dwater_base DESTINATION lib)

FILE(GLOB HEADERS "*.h")

INSTALL(FILES ${HEADERS} DESTINATION include/dwater/base)

IF(DWATER_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(tests)
ENDIF()
