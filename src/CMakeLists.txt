#
# This file is part of gbemu project.
#
# Copyright (C) 2007 FROUIN Jean-Michel
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# 02/11/07 : Adding a build counter
# 04/09/07 : Adding a cscope rule
# 17/05/08 : Adding profiling code
# 28/05/08 : Adding FindAPT.cmake

PROJECT(gbemu)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##Others dependencies for gbemu
#FOREACH(LIB zlibK)
#FIND_LIBRARY(${LIB}dev NAMES wx${LIB}${_DBG} PATHS ${WX_LIB_DIR} NO_DEFAULT_PATH)
#ENDFOREACH(LIB)

INCLUDE(FindDoxygen)
INCLUDE(FindLATEX)
INCLUDE(FindUnixCommands)
#INCLUDE(FindwxWidgets)
#INCLUDE(FindBZip2)
#INCLUDE(cmake/FindAPT.cmake)

SET(GBEMU_CFLAGS "-pg ")
SET(GBEMU_LDFLAGS "-pg ")

IF(PROFILE)
  SET(GBEMU_CFLAGS "-pg ${GBEMU_CFLAGS}")
  SET(GBEMU_LDFLAGS "-pg ${GBEMU_LDFLAGS}")
ENDIF(PROFILE)

#ADD_DEFINITIONS(-Wall -MD -fPIC -pg -g -DLEAK_DETECTOR `wx-config --cxxflags` -DwxUSE_UNICODE -DDEBUG -DVERBOSE)
ADD_DEFINITIONS(-Wall -MD -fPIC -pg -g -DLEAK_DETECTOR -DDEBUG -DVERBOSE -DPLATFORM_OS_LINUX -DWITH_SDL)

INCLUDE_DIRECTORIES(AFTER SYSTEM . ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ) #/usr/include/gtk-2.0 /usr/lib/gtk-2.0/include /usr/include/atk-1.0 /usr/include/cairo /usr/include/pango-1.0 /usr/include/glib-2.0 /usr/lib/glib-2.0/include /usr/include/freetype2 /usr/include/libpng12 /usr/include/pixman-1 /usr/include/glib-2.0 /usr/lib/glib-2.0/include)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#For installation
#SET(BINDIR /usr/bin/)
#SET(SHAREDIR /usr/share/doc/gbemu/)
#SET(SHAREAPP /usr/share/applications/)
#SET(PLUGSDIR /usr/share/gbemu/plugs/)
#SET(MANDIR /usr/share/man/man1/)
#SET(ICONS /usr/share/pixmaps/)
#SET(LANG_FR /usr/share/locale/fr/LC_MESSAGES/)

#SET(folder engine external interface leak log plugins tools)
#FOREACH(fold ${folder})
#        ADD_SUBDIRECTORY(${fold})
#ENDFOREACH(fold ${folder})

FILE(GLOB gbemu_src *.cpp)

ADD_EXECUTABLE(gbemu ${gbemu_src})
SET_TARGET_PROPERTIES(gbemu PROPERTIES LINK_FLAGS "-pg" COMPILE_FLAGS "-pg")
#TARGET_LINK_LIBRARIES(gbemu pthread dl engine interface leaks log plugins tools `wx-config --libs`)
TARGET_LINK_LIBRARIES(gbemu SDL)
# Valgrind test
ADD_CUSTOM_TARGET(valgrind COMMAND  valgrind -v --show-reachable=yes --leak-check=full gbemu)

#cscope
#ADD_CUSTOM_TARGET(cscope ALL COMMAND find ${CMAKE_SOURCE_DIR} -name "*.h" -o -name "*.cpp" > 
#${CMAKE_SOURCE_DIR}/cscope.files && cscope -b -f${CMAKE_SOURCE_DIR}/cscope.out)

#build counter
ADD_CUSTOM_TARGET(update_counter ALL COMMAND bash ${CMAKE_SOURCE_DIR}/update_build.sh ${CMAKE_SOURCE_DIR}/def.h)

#Install
#INSTALL(TARGETS gbemu DESTINATION ${BINDIR} PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gfx/splash.png DESTINATION ${SHAREDIR} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gbemu.1 DESTINATION ${MANDIR} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/fr/gbemu.mo DESTINATION ${LANG_FR} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gbemu.desktop DESTINATION ${SHAREAPP} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/sudo_gbemu.desktop DESTINATION ${SHAREAPP} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gfx/gbemu.png DESTINATION ${ICONS} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gfx/gbemu.xpm DESTINATION ${ICONS} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/gfx/gbemu_root.xpm DESTINATION ${ICONS} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
