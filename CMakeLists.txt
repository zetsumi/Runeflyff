CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(Runeflyff
        VERSION 0.74.4)
SET(RUNEFLYFF_VERSION ${PROJECT_VERSION})

# Find includes in corresponding build directories
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include local modules
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-modules")

MESSAGE("cmake_module_path = ${CMAKE_MODULE_PATH}")

# Must use a 32-bit compiler
TRY_RUN(RUN_RESULT_VAR IS_I386_COMPILER ${PROJECT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/ptr_size.cpp)
IF(NOT IS_I386_COMPILER)
    MESSAGE(FATAL_ERROR "You must use a 32-bit compiler!")
ENDIF()

# Check for C++11/C++0x compatibility. This is required.
INCLUDE(RequireCXX11)

IF(MINGW AND WIN32)
    GET_FILENAME_COMPONENT(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${Mingw_Path}/libgcc_s_dw2-1.dll ${Mingw_Path}/libwinpthread-1.dll ${Mingw_Path}/libstdc++-6.dll)
    MESSAGE(DEBUG "The following libs from MinGW will be included as part of 'cmake install' : ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
ENDIF()

SET(MYSQL_SUPPORT ON)

IF (MYSQL_SUPPORT)
    SET(MySQL_DIR "F:\\Programs (x86)\\MySQL\\MySQL Connector C 6.1")

    FIND_PACKAGE(MySQL REQUIRED)
    INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIRS})
ENDIF()

ADD_SUBDIRECTORY(login)
ADD_SUBDIRECTORY(world)