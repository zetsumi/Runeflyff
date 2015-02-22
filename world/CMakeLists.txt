CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(WorldServer)

FIND_LIBRARY(WS2_32_LIB  ws2_32)
FIND_LIBRARY(WINSOCK_LIB wsock32)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(../include)

ADD_EXECUTABLE(WorldServer7
	src/adat.cpp
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
	src/worldserver7.cpp
)

TARGET_LINK_LIBRARIES(WorldServer7
	${WS2_32_LIB}
	${WINSOCK_LIB}
	${PROJECT_SOURCE_DIR}/../lib/libmysql
	${PROJECT_SOURCE_DIR}/../lib/pthreadVC2
)