include(CheckFunctionExists)

check_function_exists(accept4 HAVE_ACCEPT4)
if(NOT HAVE_ACCEPT4)
  set_source_files_properties(socket_ops.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

set(net_SRCS
  acceptor.cc
  buffer.cc
  channel.cc
  connector.cc
  event_loop.cc
  event_loop_thread.cc
  event_loop_thread_pool.cc
  inet_address.cc
  poller.cc
  poller/default_poller.cc
  poller/epoll_poller.cc
  poller/poll_poller.cc
  socket.cc
  socket_ops.cc
  tcp_client.cc
  tcp_connection.cc
  tcp_server.cc
  timer.cc
  timer_queue.cc
  )

add_library(dwater_net ${net_SRCS})
target_link_libraries(dwater_net dwater_base)

install(TARGETS dwater_net DESTINATION lib)

set(HEADERS
  buffer.h
  callbacks.h
  channel.h
  endian.h
  event_loop.h
  event_loop_thread.h
  event_loop_thread_pool.h
  inet_address.h
  tcp_client.h
  tcp_connection.h
  tcp_server.h
  timerid.h
  )
install(FILES ${HEADERS} DESTINATION include/dwater/net)

add_subdirectory(http)
#add_subdirectory(inspect)

# if(MUDUO_BUILD_EXAMPLES)
  # add_subdirectory(tests)
# endif()

# if(PROTOBUF_FOUND)
  # add_subdirectory(protobuf)
  # add_subdirectory(protorpc)
# else()
  # add_subdirectory(protobuf EXCLUDE_FROM_ALL)
  # add_subdirectory(protorpc EXCLUDE_FROM_ALL)
# endif()
