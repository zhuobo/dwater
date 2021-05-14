set(base_SRCS
    async_logging.cc
    count_down_latch.cc
    date.cc
    file_util.cc
    logging.cc
    process_info.cc
    timestamp.cc
    condition.cc
    current_thread.cc
    exception.cc
    log_file.cc
    log_stream.cc
    thread.cc
    thread_pool.cc
    time_zone.cc
    )

add_library(dwater_base ${base_SRCS})
target_link_libraries(dwater_base pthread rt)

install(TARGETS dwater_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/dwater/base)

if(DWATER_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif()
