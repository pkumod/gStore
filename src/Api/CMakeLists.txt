include_directories( ${INCLUDE_3RD_PATH} )
add_library( httputil HttpUtil.cpp WFHttpUtil.cpp APIConnector.cpp)
add_library( api APIUtil.cpp)
