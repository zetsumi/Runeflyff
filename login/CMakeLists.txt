CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(LoginServer)

FIND_LIBRARY(WS2_32_LIB  ws2_32)
#FIND_LIBRARY(WINSOCK_LIB wsock32)

SET(MSVC_BUILD OFF)

IF(WIN32)
	IF(MSVC)
		SET(MSVC_BUILD ON)
		SET(MYSQL_LIB "libmysql")
	ELSE()
		SET(MYSQL_LIB "libmysql.lib")
	ENDIF()	
ENDIF()

CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/../include/config.h.in" 
		"${PROJECT_BINARY_DIR}/config.h"
)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

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
	#${WINSOCK_LIB}
	${PROJECT_SOURCE_DIR}/../lib/${MYSQL_LIB}
)