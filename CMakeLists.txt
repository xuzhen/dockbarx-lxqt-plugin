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

set(QT5_COMPONENTS Core Gui DBus Widgets X11Extras)
find_package(Qt5 COMPONENTS ${QT5_COMPONENTS} REQUIRED)

set(QT5_INCLUDE_DIRS "")
set(QT5_LIBRARIES "")
foreach(temp ${QT5_COMPONENTS})
    list(APPEND QT5_INCLUDE_DIRS "${Qt5${temp}_INCLUDE_DIRS}")
    list(APPEND QT5_LIBRARIES "${Qt5${temp}_LIBRARIES}")
endforeach()

include_directories(${QT5_INCLUDE_DIRS})

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
)
set(PLUGIN_HDR
    src/configdialog.h
    src/dbusproxy.h
    src/lxqtplugin.h
    src/lxqtpluginlibrary.h
    src/panelsettings.h
    src/panelsettingswatcher.h
    src/pyappletkeeper.h
)

add_library(plugin MODULE ${PLUGIN_HDR} ${PLUGIN_SRC})
target_link_libraries(plugin ${QT5_LIBRARIES})
set_target_properties(plugin PROPERTIES OUTPUT_NAME ${PLUGIN_ID})

install(TARGETS plugin LIBRARY
        DESTINATION ${PLUGIN_LIB_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/lxqt-panel-applet.py
        DESTINATION ${DOCKBARX_PATH}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/dockbarx.desktop
        DESTINATION ${PLUGIN_DESKTOP_DIR})
