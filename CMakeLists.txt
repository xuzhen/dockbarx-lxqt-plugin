cmake_minimum_required(VERSION 3.7.0)
project(dockbarx-lxqt-plugin)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

set(PLUGIN_ID "dockbarx")
set(PLUGIN_VERSION "0.9")

set(PLUGIN_LIB_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/lxqt-panel/")
set(PLUGIN_DESKTOP_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxqt/lxqt-panel/")
set(DOCKBARX_PATH "${CMAKE_INSTALL_FULL_DATAROOTDIR}/dockbarx")

execute_process(
    COMMAND sh -c "lxqt-panel --version | grep ^Qt | sed -E 's/^Qt\\s*([0-9]+)\\..*$/\\1/' | tr -d $'\\n'"
    ERROR_QUIET
    OUTPUT_VARIABLE QT_VER
)
if(NOT QT_VER)
    find_package(Qt6 QUIET COMPONENTS Core)
    if (NOT Qt6_FOUND)
        find_package(Qt5 COMPONENTS Core REQUIRED)
        set(QT_VER 5)
    else()
        set(QT_VER 6)
    endif()
    message(WARNING "Can not detect Qt version from lxqt-panel. Try to build with Qt ${QT_VER}")
endif()

set(QT_COMPONENTS Core Gui DBus Widgets)
if(QT_VER EQUAL 5)
    list(APPEND QT_COMPONENTS X11Extras)
endif()

find_package(Qt${QT_VER} COMPONENTS ${QT_COMPONENTS} REQUIRED)

set(QT_INCLUDE_DIRS "")
set(QT_LIBRARIES "")
foreach(temp ${QT_COMPONENTS})
    list(APPEND QT_INCLUDE_DIRS "${Qt${QT_VER}${temp}_INCLUDE_DIRS}")
    list(APPEND QT_LIBRARIES "${Qt${QT_VER}${temp}_LIBRARIES}")
endforeach()

include_directories(${QT_INCLUDE_DIRS})

foreach(temp ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    if (EXISTS ${temp}/lxqt/ilxqtpanel.h AND EXISTS ${temp}/lxqt/ilxqtpanelplugin.h)
        set(HAVE_LXQTPANEL_HEADERS TRUE)
        break()
    endif()
endforeach()
if(NOT "${HAVE_LXQTPANEL_HEADERS}")
    message(FATAL_ERROR "LXQt panel header files are not found" )
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions("-DUNICODE")

configure_file(src/config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PLUGIN_SRC
    src/configdialog.cpp
    src/dbusproxy.cpp
    src/lxqtplugin.cpp
    src/lxqtpluginlibrary.cpp
    src/panelsettings.cpp
    src/panelsettingswatcher.cpp
    src/pyappletkeeper.cpp
    src/dockbarcontainer.cpp
)
set(PLUGIN_HDR
    src/configdialog.h
    src/dbusproxy.h
    src/lxqtplugin.h
    src/lxqtpluginlibrary.h
    src/panelsettings.h
    src/panelsettingswatcher.h
    src/pyappletkeeper.h
    src/dockbarcontainer.h
)

add_library(plugin MODULE ${PLUGIN_HDR} ${PLUGIN_SRC})
target_link_libraries(plugin ${QT_LIBRARIES})
set_target_properties(plugin PROPERTIES OUTPUT_NAME ${PLUGIN_ID})

install(TARGETS plugin LIBRARY
        DESTINATION ${PLUGIN_LIB_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/lxqt-panel-applet.py
        DESTINATION ${DOCKBARX_PATH}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/dockbarx.desktop
        DESTINATION ${PLUGIN_DESKTOP_DIR})
