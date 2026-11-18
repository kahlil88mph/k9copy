cmake_minimum_required(VERSION 3.20)
project(k9copy VERSION 3.1.0 LANGUAGES C CXX)
set(VERSION ${PROJECT_VERSION})

# Plain Qt6 build: no KDE Frameworks. Video preview via libmpv (or the
# built-in libmpeg2 viewer); drive detection via libudev; translations via
# gettext. See git history for the original KDE4/KF5 build.

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
include(GNUInstallDirs)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Xml DBus OpenGL OpenGLWidgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DVDREAD REQUIRED IMPORTED_TARGET dvdread)
pkg_check_modules(DVDNAV  REQUIRED IMPORTED_TARGET dvdnav)
pkg_check_modules(MPEG2   REQUIRED IMPORTED_TARGET libmpeg2 libmpeg2convert)
pkg_check_modules(MPV     REQUIRED IMPORTED_TARGET mpv)
pkg_check_modules(UDEV    REQUIRED IMPORTED_TARGET libudev)
pkg_check_modules(FFMPEG  REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
                -DQT_NO_DEPRECATED_WARNINGS
                -DK9_LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
                -DK9_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/k9copy")

include(CheckIncludeFiles)
foreach(_h inttypes stddef stdint stdlib string unistd malloc)
    string(TOUPPER ${_h} _H)
    check_include_files(${_h}.h HAVE_${_H}_H)
endforeach()
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
set(HAVE_SWSCALE 1)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# uic output uses our gettext-backed tr2i18n() instead of QObject::tr().
macro(k9_wrap_ui outvar)
    qt_wrap_ui(${outvar} ${ARGN} OPTIONS -tr tr2i18n -include k9i18n.h --connections string)
endmacro()

add_subdirectory(icons)
add_subdirectory(data)
add_subdirectory(po)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compat
    ${CMAKE_CURRENT_SOURCE_DIR}/src/devices
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vamps
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backup
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mpeg2
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mplayer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assistant
    ${CMAKE_CURRENT_SOURCE_DIR}/src/import
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player
    ${CMAKE_CURRENT_SOURCE_DIR}/src/subtitles
    ${CMAKE_CURRENT_SOURCE_DIR}/src/batch
    ${DVDREAD_INCLUDEDIR}/dvdread
)

set(core_SRCS
    src/core/k9dvd.cpp src/core/k9dvdtitle.cpp src/core/k9dvdtitleset.cpp
    src/core/k9dvdchapter.cpp src/core/k9dvdsize.cpp src/core/k9process.cpp
    src/core/k9tools.cpp src/core/k9dvdread.cpp src/core/k9cellcopylist.cpp
    src/core/k9cell.cpp src/core/k9dvdprogress.cpp src/core/k9ifo2.cpp
    src/core/k9titleencopt.cpp src/core/k9config.cpp src/core/k9saveimage.cpp
    src/core/k9drawimage.cpp src/core/k9progress.cpp src/core/k9burnprogress.cpp
    src/core/k9audiocodecs.cpp src/core/k9videocodecs.cpp src/core/k9burndvd.cpp
    src/core/k9dvdbackupinterface.cpp src/core/k9dialogs.cpp src/core/k9log.cpp
    src/core/k9logview.cpp)
k9_wrap_ui(core_SRCS src/core/dvdprogress.ui src/core/progress.ui src/core/k9logview.ui)

set(backup_SRCS
    src/backup/k9dvdbackup.cpp src/backup/k9script.cpp src/backup/k9backupdlg.cpp
    src/backup/k9dvdauthor.cpp src/backup/k9mp4enc.cpp src/backup/k9mp4dlg.cpp
    src/backup/k9execcopy.cpp src/backup/k9convertaudio.cpp src/player/k9play.cpp)
k9_wrap_ui(backup_SRCS src/backup/backupdlg.ui src/backup/mp4dlg.ui)

set(mpeg2_SRCS src/mpeg2/k9decodethread.cpp src/mpeg2/k9plaympeg2.cpp src/mpeg2/kdecmpeg2.cpp)
set(vamps_SRCS src/vamps/k9fifo.cpp src/vamps/k9requant.cpp src/vamps/k9vamps.cpp)
set(subtitles_SRCS src/subtitles/k9subtitle2pgm.cpp)
set(devices_SRCS src/devices/k9cddrive.cpp)

set(mplayer_SRCS src/mplayer/k9mpv.cpp)
k9_wrap_ui(mplayer_SRCS src/mplayer/phonon.ui)

set(import_SRCS
    src/import/k9chapteredit.cpp src/import/k9avidecode.cpp src/import/k9avifile.cpp
    src/import/k9import.cpp src/import/k9lvitemimport.cpp src/import/k9menubutton.cpp
    src/import/k9menu.cpp src/import/k9menuedit.cpp src/import/k9menuscene.cpp
    src/import/k9newdvd.cpp src/import/k9newtitle.cpp src/import/k9title.cpp
    src/import/k9processlist.cpp src/import/k9importfiles.cpp)
k9_wrap_ui(import_SRCS
    src/import/chapterEdit.ui src/import/import.ui src/import/menuEdit.ui
    src/import/newTitle.ui src/import/processList.ui)

set(assistant_SRCS
    src/assistant/k9assistant.cpp src/assistant/k9astsource.cpp
    src/assistant/k9astdestination.cpp src/assistant/k9asttitles.cpp
    src/assistant/k9aststreams.cpp src/assistant/k9astmp4.cpp
    src/assistant/k9astmpeg.cpp src/assistant/k9astdvdoption.cpp)
k9_wrap_ui(assistant_SRCS
    src/assistant/astsource.ui src/assistant/astdestination.ui src/assistant/asttitles.ui
    src/assistant/aststreams.ui src/assistant/astmp4.ui src/assistant/astmpeg.ui
    src/assistant/astdvdoption.ui)

set(main_SRCS
    src/main/k9main.cpp src/main/k9langselect.cpp src/main/k9playbackoptions.cpp
    src/main/k9updatefactor.cpp src/main/kviewmpeg2.cpp src/main/k9cropselect.cpp
    src/main/k9titlefactor.cpp src/main/k9mp4title.cpp src/main/k9settings.cpp
    src/main/k9copy.cpp src/main/kconfigdlg.cpp src/main/k9prefdvd.cpp
    src/main/k9prefmencoder.cpp src/main/k9mencodercmdgen.cpp src/main/k9ffmpegcmdgen.cpp
    src/main/k9prefmpeg4.cpp src/main/k9prefpreview.cpp src/main/k9prefauthor.cpp
    src/main/k9prefdirs.cpp src/batch/k9batch.cpp)
k9_wrap_ui(main_SRCS
    src/main/k9mainw.ui src/main/configDlg.ui src/main/playbackoptionsw.ui
    src/main/prefDVD.ui src/main/prefMPEG4.ui src/main/prefMencoder.ui
    src/main/prefAuthor.ui src/main/prefpreview.ui src/main/langselectw.ui
    src/main/viewmpeg2.ui src/main/titlefactor.ui src/main/mencoderCmdGen.ui
    src/main/ffmpegCmdGen.ui src/main/prefDirs.ui)

file(GLOB compat_SRCS src/compat/*.cpp)

add_library(k9copylib STATIC ${compat_SRCS} ${core_SRCS} ${vamps_SRCS} ${mpeg2_SRCS} ${subtitles_SRCS} ${backup_SRCS})
target_link_libraries(k9copylib PUBLIC
    PkgConfig::DVDREAD PkgConfig::DVDNAV PkgConfig::MPEG2 PkgConfig::FFMPEG PkgConfig::UDEV
    Qt6::Widgets Qt6::Xml Qt6::DBus ${CMAKE_DL_LIBS})

qt_add_resources(k9copy_RCC k9copy.qrc)
add_executable(k9copy ${k9copy_RCC} ${devices_SRCS} ${mplayer_SRCS} ${import_SRCS} ${main_SRCS} ${assistant_SRCS} main.cpp)
target_link_libraries(k9copy k9copylib PkgConfig::MPV Qt6::OpenGL Qt6::OpenGLWidgets)

add_executable(k9play src/player/main.cpp)
target_link_libraries(k9play k9copylib)

install(TARGETS k9copy k9play RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES k9copy.desktop k9copy_assistant.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES k9copy_open.desktop k9copy_assistant_open.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/solid/actions)
foreach(_sz 16 22 32 48)
    install(FILES ${_sz}-apps-k9copy.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${_sz}x${_sz}/apps RENAME k9copy.png)
endforeach()
