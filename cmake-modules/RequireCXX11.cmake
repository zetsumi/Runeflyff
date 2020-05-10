INCLUDE(CXXCompilerLevels)

IF(NOT COMPILER_SUPPORTS_CXX11)
    MESSAGE(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
ENDIF()