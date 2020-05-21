CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(WorldServer)

SET(MSVC_BUILD OFF)

IF(WIN32)
	IF(MSVC)
		SET(MSVC_BUILD ON)
		SET(PTHREAD_DLL "${PROJECT_SOURCE_DIR}/../dll/pthreadVC2.dll")
		SET(PTHREAD_LIB "${PROJECT_SOURCE_DIR}/../lib/pthreadVC2.lib")
	ELSE()
		SET(THREADS_PREFER_PTHREAD_FLAG ON)
		FIND_PACKAGE(Threads REQUIRED)
		SET(PTHREAD_LIB Threads::Threads)
	ENDIF()
ENDIF()

CONFIGURE_FILE(
		"${PROJECT_SOURCE_DIR}/../include/config.h.in"
		"${PROJECT_BINARY_DIR}/config.h"
)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

ADD_EXECUTABLE(WorldServer7
		src/adat.cpp
		src/adat_constants.cpp
		src/attack.cpp
		src/bank.cpp
		src/buff.cpp
		src/buffer.cpp
		src/character_base.cpp
		src/character_npc.cpp
		src/character_obj.cpp
		src/charserver.cpp
		src/cluster.cpp
		src/fieldobj.cpp
		src/funkciok.cpp
		src/grid.cpp
		src/gridremover.cpp
		src/guild.cpp
		src/inventory.cpp
		src/item.cpp
		src/logger.cpp
		src/main.cpp
		src/monster.cpp
		src/network.cpp
		src/party.cpp
		src/player.cpp
		src/playershop.cpp
		src/player_process.cpp
		src/quest.cpp
		src/reciever.cpp
		src/slot.cpp
		src/spawns.cpp
		src/stringf.cpp
		src/trade.cpp
		src/worldserver7.cpp)

TARGET_LINK_LIBRARIES(WorldServer7
		ws2_32
		wsock32
		${PTHREAD_LIB})

SET(PROJECT_INSTALL_DIR ${PROJECT_BINARY_DIR}/../dist/${CMAKE_BUILD_TYPE})

IF (MYSQL_SUPPORT)
	TARGET_LINK_LIBRARIES(WorldServer7 ${MYSQL_LIBRARIES})

	IF (MSVC)
		INSTALL(FILES ${MYSQL_DYNAMIC} DESTINATION ${PROJECT_INSTALL_DIR}/bin)
	ELSE()
		INSTALL(FILES ${MYSQL_LIBRARIES} DESTINATION ${PROJECT_INSTALL_DIR}/bin)
	ENDIF()
ENDIF()

INSTALL(TARGETS WorldServer7
		RUNTIME DESTINATION ${PROJECT_INSTALL_DIR}/bin)

INSTALL(DIRECTORY resources DESTINATION ${PROJECT_INSTALL_DIR}/bin)

IF(MSVC)
	INSTALL(FILES
			${PTHREAD_DLL}
			DESTINATION ${PROJECT_INSTALL_DIR}/bin)
ENDIF()