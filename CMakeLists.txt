# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Please note that the package source code is licensed under its own license.

project ( rxspencer C )
cmake_minimum_required ( VERSION 3.0 )
include ( cmake/mkh.cmake )
option ( rxshared "build shared library instead of static" OFF )

# Patch regerror.c to avoid conflict in MSVC
if ( MSVC )
  file ( READ regerror.c REGERROR )
  string ( REGEX MATCH errcode REGERROR_UNPATCHED "${REGERROR}" )
  if ( REGERROR_UNPATCHED )
    string ( REPLACE errcode errorcode REGERROR "${REGERROR}" )
    file ( WRITE regerror.c "${REGERROR}" )
  endif ( )
endif ( )

mkh ( regcomp.c ${CMAKE_BINARY_DIR}/regcomp.ih "" )
mkh ( engine.c ${CMAKE_BINARY_DIR}/engine.ih "" )
mkh ( regerror.c ${CMAKE_BINARY_DIR}/regerror.ih "" )
mkh ( debug.c ${CMAKE_BINARY_DIR}/debug.ih "" )
mkh ( main.c ${CMAKE_BINARY_DIR}/main.ih "" )
mkh ( "regex2.h;regcomp.c;regerror.c;regexec.c;regfree.c" ${CMAKE_BINARY_DIR}/regex.h "_REGEX_H_" )

# Patch regex.h for off_t on msvc
if ( MSVC )
  file ( READ ${CMAKE_BINARY_DIR}/regex.h REGEXH )
  string ( REGEX MATCH "sys/types" REGEXH_PATCHED "${REGEXH}" )
  if ( NOT REGEXH_PATCHED )
    string ( REPLACE "typedef off_t" "#include <sys/types.h>\ntypedef off_t" REGEXH
      "${REGEXH}" )
    file ( WRITE "${CMAKE_BINARY_DIR}/regex.h" "${REGEXH}" )
    set ( REGEX_H ${CMAKE_BINARY_DIR}/regex.h )
  endif ( )
endif ( )

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

set_target_properties ( rxspencer PROPERTIES PUBLIC_HEADER ${CMAKE_BINARY_DIR}/regex.h VERSION 0.0.3 SOVERSION 0)
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
