# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Please note that the package source code is licensed under its own license.

project ( rxspencer C )
cmake_minimum_required ( VERSION 3.0 )
option ( rxshared "build shared library instead of static" OFF )
enable_testing()

# Define POSIX_MISTAKE to allow unmatched right parentheses as literals, as
# required by POSIX 1003.2.
# Define REDEBUG to enable debugging.
add_definitions(-DPOSIX_MISTAKE -DREDEBUG)
#add_compile_options(-Wall -Wextra -pedantic -Wconversion)

# rxspencer library
set ( CMAKE_INCLUDE_CURRENT_DIR 1 )
set ( RXSPENCER_SRCS regcomp.c regerror.c regexec.c regfree.c )
set ( RXSPENCER_INCLUDE_DIR include/rxspencer )
set ( RXSPENCER_BIN_DIR bin )
set ( RXSPENCER_LIB_DIR lib )
set ( RXSPENCER_DATA_DIR share/rxspencer )
set ( RXSPENCER_MAN3_DIR man/man3 )
set ( RXSPENCER_MAN7_DIR man/man7 )

if ( MSVC )
  set ( DEF_FILE librxspencer.def )
endif ( )

if ( rxshared )
  add_library ( rxspencer SHARED ${RXSPENCER_SRCS} ${DEF_FILE} )
else ( )
  add_library ( rxspencer STATIC ${RXSPENCER_SRCS} ${DEF_FILE} )
endif ( )

set_target_properties ( rxspencer PROPERTIES PUBLIC_HEADER regex.h VERSION 0.0.3 SOVERSION 0)
target_include_directories ( rxspencer PUBLIC
  $<INSTALL_INTERFACE:${RXSPENCER_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}> )

install ( TARGETS rxspencer
  EXPORT RXSpencerTargets
  RUNTIME DESTINATION ${RXSPENCER_BIN_DIR}
  LIBRARY DESTINATION ${RXSPENCER_LIB_DIR}
  ARCHIVE DESTINATION ${RXSPENCER_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${RXSPENCER_INCLUDE_DIR} )

# Install docs
set ( RXSPENCER_DATA_FILES "COPYRIGHT;README;WHATSNEW" )
install ( FILES ${RXSPENCER_DATA_FILES} DESTINATION ${RXSPENCER_DATA_DIR} )
install ( FILES rxspencer.3 DESTINATION ${RXSPENCER_MAN3_DIR} )
install ( FILES rxspencer.7 DESTINATION ${RXSPENCER_MAN7_DIR} )

# generate cmake configuration file
include ( CMakePackageConfigHelpers )
if ( NOT DEFINED CMAKE_CONFIG_DEST )
  set ( CMAKE_CONFIG_DEST lib/cmake/rxspencer )
endif()
configure_package_config_file (
  RXSpencerConfig.cmake.in
  ${CMAKE_BINARY_DIR}/RXSpencerConfig.cmake
  INSTALL_DESTINATION ${CMAKE_CONFIG_DEST}
  PATH_VARS RXSPENCER_INCLUDE_DIR CMAKE_CONFIG_DEST )

install ( EXPORT RXSpencerTargets DESTINATION ${CMAKE_CONFIG_DEST} )

# install the configuration file
install ( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/RXSpencerConfig.cmake
  DESTINATION ${CMAKE_CONFIG_DEST} )

add_executable(tester ${RXSPENCER_SRCS} debug.c main.c split.c)
add_test(NAME test1 COMMAND sh -c "./tester -f ${CMAKE_CURRENT_SOURCE_DIR}/tests")
add_test(NAME test2 COMMAND sh -c "./tester -el -f ${CMAKE_CURRENT_SOURCE_DIR}/tests")
add_test(NAME test3 COMMAND sh -c "./tester -er -f ${CMAKE_CURRENT_SOURCE_DIR}/tests")
