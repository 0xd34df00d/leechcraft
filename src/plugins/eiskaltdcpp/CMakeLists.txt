set (PROJECT_NAME_GLOBAL leechcraft_eiskaltdcpp)
IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_eiskaltdcpp)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckFunctionExists)
include (FindPkgConfig)
include (CheckCXXSourceCompiles)

CHECK_INCLUDE_FILE (malloc.h HAVE_MALLOC_H)
CHECK_INCLUDE_FILE (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE (inttypes.h HAVE_INTTYPES_H)
CHECK_FUNCTION_EXISTS(mallinfo HAVE_MALLINFO)
CHECK_FUNCTION_EXISTS(malloc_stats HAVE_MALLOC_STATS)
CHECK_FUNCTION_EXISTS(malloc_trim HAVE_MALLOC_TRIM)
CHECK_FUNCTION_EXISTS(daemon HAVE_DAEMON)
CHECK_INCLUDE_FILE (memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE (stdbool.h HAVE_STDBOOL_H)
CHECK_INCLUDE_FILE (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE (sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE (limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE (sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE (ifaddrs.h HAVE_IFADDRS_H)
CHECK_INCLUDE_FILE (net/if.h HAVE_NET_IF_H)
CHECK_INCLUDE_FILE (sys/statvfs.h STAT_STATVFS)
CHECK_INCLUDE_FILES ("sys/socket.h;net/if.h;ifaddrs.h;sys/types.h" HAVE_ADDRS_H)
CHECK_INCLUDE_FILES ("sys/types.h;sys/statvfs.h;limits.h;stdbool.h;stdint.h" FS_USAGE_C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (X11)
find_package (PkgConfig)
find_package (BZip2 REQUIRED)
find_package (ZLIB REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (Threads REQUIRED)
find_package (Gettext REQUIRED)
find_package (Iconv REQUIRED)
find_package (Git)
find_package (Boost REQUIRED)

option (USE_QT "Qt interface" ON)
option (USE_QT_QML "Build with Qt Declarative Ui support" OFF)
option (USE_QT_SQLITE "Build with SQLite support" OFF)
option (USE_MINIUPNP "Use miniupnpc for forward ports" ON)
option (LOCAL_MINIUPNP "Use local miniupnpc" ON)
option (USE_ASPELL "Use aspell for spellchecking")
option (FORCE_XDG "Respect freedesktop.org standards" ON)
option (FREE_SPACE_BAR_C "Free space progressbar in Qt interface" ON)
option (WITH_EMOTICONS "Install emoticon packs" ON)
option (WITH_EXAMPLES "Install examples sh and others scripts" ON)
option (DBUS_NOTIFY "QtDbus support in Qt interface" ON)
option (USE_JS "QtScript support in Qt interface" ON)

if (USE_MINIUPNP)
    message (STATUS "Building with UPNP support (miniupnpc)")

  if (LOCAL_MINIUPNP)
    add_subdirectory(miniupnpc)
    set (MINIUPNP_INCLUDE_DIR "./")
    set (MINIUPNP_LIBRARY "miniupnpc")
  else (LOCAL_MINIUPNP)
    find_package (Miniupnpc REQUIRED)
  endif (LOCAL_MINIUPNP)

elseif (NOT USE_MINIUPNP)
    unset (MINIUPNP_INCLUDE_DIR CACHE)
    unset (MINIUPNP_LIBRARY CACHE)
    message (STATUS "Building without UPNP support (miniupnpc)")
endif(USE_MINIUPNP)

if (USE_QT)
    if (USE_QT_QML)
      message (STATUS "Building with Qt Declarative UI support")
      set (USE_JS ON)
    else (USE_QT_QML)
      message (STATUS "Building without Qt Declarative UI support")
    endif (USE_QT_QML)

    if (DBUS_NOTIFY)
        message (STATUS "Building with Qt DBus support")
    else (DBUS_NOTIFY)
        message (STATUS "Building without Qt DBus support")
    endif (DBUS_NOTIFY)

    if (USE_JS)
	message (STATUS "Building with Qt Script support")
    else (USE_JS)
        message (STATUS "Building without Qt Script support")
    endif (USE_JS)

    if (USE_QT_SQLITE)
        message (STATUS "Building with Qt SQLite support")
    else (USE_QT_SQLITE)
        message (STATUS "Building without Qt SQLite support")
    endif (USE_QT_SQLITE)
endif (USE_QT)

if (NOT USE_ASPELL)
    if(USE_ASPELL EQUAL 0)
    else(USE_ASPELL EQUAL 0)
        find_package (ASPELL)
        if (ASPELL_FOUND)
            set (USE_ASPELL 1 CACHE BOOL "Use aspell for spellchecking" FORCE)
        endif (ASPELL_FOUND)
    endif(USE_ASPELL EQUAL 0)
endif (NOT USE_ASPELL)

if (USE_ASPELL)
    find_package (ASPELL REQUIRED)
    message (STATUS "Building with Aspell support")
else (USE_ASPELL)
    message (STATUS "Building without Aspell support")
endif (USE_ASPELL)

if (FREE_SPACE_BAR_C)
    message (STATUS "Building with free space bar (realisation on C, POSIX 1003.1-2001 (and later) with XSI)")
else (FREE_SPACE_BAR_C)
    message (STATUS "Building without free space bar")
endif (FREE_SPACE_BAR_C)


find_package (Boost REQUIRED)
if (Boost_VERSION VERSION_LESS 103900)
  add_definitions(-DFIX_FOR_OLD_BOOST)
endif()
if (Boost_VERSION VERSION_LESS 104200)
  add_definitions(-DTIMER_OLD_BOOST)
endif()

if (NOT CMAKE_INSTALL_PREFIX)
  set (CMAKE_INSTALL_PREFIX "/usr")
endif (NOT CMAKE_INSTALL_PREFIX)

if (LIB_INSTALL_DIR)
  set (LIBDIR ${LIB_INSTALL_DIR})
endif (LIB_INSTALL_DIR)

if (NOT LIBDIR)
  set (LIBDIR "lib")
endif (NOT LIBDIR)
message (STATUS "Library directory: ${CMAKE_INSTALL_PREFIX}/${LIBDIR}")

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND USE_QT)
  message(STATUS "EiskaltDC++ Qt with CMAKE_BUILD_TYPE=Debug is very unstable now!\n"
                 "\tYou can build it for your own risk.\n"
                 "\tBut do not even try to write bug reports in this case.\n"
                 "\tPlease use RelWithDebInfo instead it.")
elseif (CMAKE_BUILD_TYPE STREQUAL "Gentoo")
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
  message(STATUS "CMAKE_BUILD_TYPE = Gentoo is not now supported. RelWithDebInfo is used instead.")
endif (NOT CMAKE_BUILD_TYPE)

if (NOT SHARE_DIR)
  set (SHARE_DIR "share")
endif (NOT SHARE_DIR)

if (APPLE)
  set (CLIENT_DATA_DIR "${PROJECT_NAME_GLOBAL}.app")
elseif (WIN32)
  set (CLIENT_DATA_DIR "${SHARE_DIR}")
else (APPLE)
  set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/leechcraft/eiskaltdcpp")
endif (APPLE)

if (APPLE)
  set (LOCALE_DIR "${PROJECT_NAME_GLOBAL}.app/locale")
elseif (WIN32)
  set (LOCALE_DIR "${SHARE_DIR}/locale")
else (APPLE)
  set (LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/${SHARE_DIR}/leechcraft/translations")
endif (APPLE)

set (_DATADIR "${CLIENT_DATA_DIR}") # don't change! (used by eiskaltdcpp-gtk)

message (STATUS "Data directory: ${CLIENT_DATA_DIR}")

if (DCPP_REVISION)
    set (eiskaltdcpp_WC_REVISION ${DCPP_REVISION})
    message (STATUS "DCPP_REVISION is set to: ${eiskaltdcpp_WC_REVISION}")
elseif (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                OUTPUT_VARIABLE eiskaltdcpp_WC_REVISION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
    message (STATUS "Build Git commit ID: ${eiskaltdcpp_WC_REVISION}")
    if (NOT eiskaltdcpp_WC_REVISION MATCHES [0-9a-z]+)
        set (eiskaltdcpp_WC_REVISION "${VERSION_SUFFIX}")
    endif ()
elseif (NOT DCPP_REVISION)
    set (eiskaltdcpp_WC_REVISION "${VERSION_SUFFIX}")
endif ()

include_directories (${CMAKE_CURRENT_BINARY_DIR} ${LEECHCRAFT_INCLUDE_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/VersionGlobal.h)

if (UNIX)
  add_definitions (-D_GNU_SOURCE)
endif (UNIX)

set (CLIENT_TRANSLATIONS_DEST "${CLIENT_DATA_DIR}/qt/ts")
set (CLIENT_SOUNDS_DEST "${CLIENT_DATA_DIR}/sounds")
set (CLIENT_ICONS_DEST "${CLIENT_DATA_DIR}/icons")
set (CLIENT_SCRIPTS_DEST "${CLIENT_DATA_DIR}/qt/qtscripts")

add_definitions (-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT
                 -D_DATADIR="${_DATADIR}" -DLOCALE_DIR="${LOCALE_DIR}" -DBUILDING_DCPP)

if (ENABLE_STACKTRACE)
    add_definitions(-DENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

if (HAVE_IFADDRS_H)
  add_definitions (-DHAVE_IFADDRS_H )
endif (HAVE_IFADDRS_H)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    if (MINGW)
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-auto-import")
    endif (MINGW)
    if (NOT APPLE)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--as-needed")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed")
    endif (NOT APPLE)
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pipe -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE     "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG -pipe")
    set (CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -UNDEBUG -g3 -pipe")
endif ()

include_directories(${eiskaltdcpp_BINARY_DIR}
   ${eiskaltdcpp_SOURCE_DIR}
   )

if (UNIX AND NOT APPLE)
  set (CLIENT_ICONS_DEST1 ${SHARE_DIR}/icons)
  set (CLIENT_ICONS_DEST2 ${SHARE_DIR}/${PROJECT_NAME_GLOBAL}/icons)
else ()
  set (CLIENT_ICONS_DEST1 ${CLIENT_ICONS_DEST})
  set (CLIENT_ICONS_DEST2 ${CLIENT_ICONS_DEST})
endif ()

option (LANGUAGES "Using translations")
list (APPEND AllLanguages
            be
            bg
            cs
            en
            es
            fr
            hu
            pl
            ru
            sr
            sk
            uk
            )
if (NOT DEFINED linguas)
  set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
else (NOT DEFINED linguas)
  if (NOT linguas)
    set (LANGUAGES "" CACHE STRING "Using translations" FORCE)
  elseif (linguas STREQUAL *)
        set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
  else (NOT linguas)
    string(REGEX MATCHALL [a-zA-Z_]+
           linguas1 ${linguas})
    set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
  endif (NOT linguas)
endif (NOT DEFINED linguas)
message (STATUS "Translations: ${LANGUAGES}")

add_subdirectory(extra)

add_subdirectory (dcpp)
add_subdirectory (dht)
add_subdirectory (eiskaltdcpp-qt)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

if(GETTEXT_FOUND)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/KWWidgetsInternationalizationMacros.cmake)
    option (UPDATE_PO "Update po files" OFF)
    option (CREATE_MO "Create mo files" ON)

    if (UPDATE_PO)
        set (create_pot 1)
        set (create_po 1)
        set (create_po_lo 1)
    else (UPDATE_PO)
        set (create_pot 0)
        set (create_po 0)
        set (create_po_lo 0)
    endif (UPDATE_PO)
    if (CREATE_MO)
        set (create_mo 1)
        set (create_mo_lo 1)
    else (CREATE_MO)
        set (create_mo 0)
        set (create_mo_lo 0)
    endif (CREATE_MO)
    set (mo_for_all 1)
    kwwidgets_create_gettext_targets(
      DOMAIN_NAME "libeiskaltdcpp"
      LOCALE_LIST "${LANGUAGES}"
      COPYRIGHT_HOLDER "EiskaltDC++ team"
      SOURCES "${DCPP_SRCS};${DCPP_HDRS}"
      PO_DIR "${dcpp_SOURCE_DIR}/po"
      POT_BUILD_DIR "${dcpp_SOURCE_DIR}/po"
      #PO_PREFIX    ""
      PO_BUILD_DIR "${dcpp_SOURCE_DIR}/po"
      MO_BUILD_DIR "${dcpp_BINARY_DIR}/locale"
      MO_INSTALL_DIR "/${LOCALE_DIR}"
      CREATE_POT_TARGET ${create_pot}
      CREATE_PO_TARGET ${create_po}
      CREATE_PO_LOCALE_TARGETS ${create_po_lo}
      CREATE_MO_TARGET ${create_mo}
      CREATE_MO_LOCALE_TARGETS ${create_mo_lo}
      ADD_MO_TARGET_TO_ALL ${mo_for_all}
      )
    if (USE_GTK)
         kwwidgets_create_gettext_targets(
          DOMAIN_NAME "eiskaltdcpp-gtk"
          LOCALE_LIST "${LANGUAGES}"
          COPYRIGHT_HOLDER "EiskaltDC++ team"
          SOURCES "${GTK_SRCS};${GTK_HDRS};${GTK_GLADE}"
          PO_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
          POT_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
          #PO_PREFIX    ""
          PO_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_SOURCE_DIR}/po"
          MO_BUILD_DIR "${${PROJECT_NAME_GLOBAL}-gtk_BINARY_DIR}/locale"
          MO_INSTALL_DIR "/${LOCALE_DIR}"
          CREATE_POT_TARGET ${create_pot}
          CREATE_PO_TARGET ${create_po}
          CREATE_PO_LOCALE_TARGETS ${create_po_lo}
          CREATE_MO_TARGET ${create_mo}
          CREATE_MO_LOCALE_TARGETS ${create_mo_lo}
          ADD_MO_TARGET_TO_ALL ${mo_for_all}
          )

    endif (USE_GTK)

endif(GETTEXT_FOUND)

if (WITH_EXAMPLES)
  install (DIRECTORY examples/ DESTINATION ${CLIENT_DATA_DIR}/examples
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  PATTERN ".svn" EXCLUDE )
  install (FILES update_geoip DESTINATION ${CLIENT_DATA_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (WITH_EXAMPLES)

if (WITH_EMOTICONS)
  install (DIRECTORY emoticons/ DESTINATION ${CLIENT_DATA_DIR}/emoticons
  PATTERN ".svn" EXCLUDE)
endif (WITH_EMOTICONS)
