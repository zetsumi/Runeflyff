CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(LoginServer)

FIND_LIBRARY(WS2_32_LIB  ws2_32)
FIND_LIBRARY(WINSOCK_LIB wsock32)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(../include)

ADD_EXECUTABLE(Login
	src/buffer.cpp
	src/logger.cpp
	src/login.cpp
	src/network.cpp
	src/PacketLogger.cpp
	src/reciever.cpp
	src/stringf.cpp
)
	

TARGET_LINK_LIBRARIES(Login
	${WS2_32_LIB}
	${WINSOCK_LIB}
	${PROJECT_SOURCE_DIR}/../lib/libmysql
)