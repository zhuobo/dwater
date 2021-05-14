set(http_SRCS
  http_server.cc
  http_response.cc
  http_context.cc
  )

add_library(dwater_http ${http_SRCS})
target_link_libraries(dwater_http dwater_net)

install(TARGETS dwater_http DESTINATION lib)
set(HEADERS
  http_server.h
  http_response.h
  http_context.h
  http_request.h
  )
install(FILES ${HEADERS} DESTINATION include/dwater/net/http)


