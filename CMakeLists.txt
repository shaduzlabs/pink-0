
        ##########    Copyright (C) 2016 Vincenzo Pacella
        ##      ##    Distributed under MIT license, see file LICENSE
        ##      ##    or <http://opensource.org/licenses/MIT>
        ##      ##
##########      ############################################################# shaduzlabs.com #######

cmake_minimum_required (VERSION 3.0)
project (pink)

set(PINK_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(PINK_BUILD_DIR ${CMAKE_BINARY_DIR})
set(PINK_MODULES_DIR ${PINK_ROOT_DIR}/modules)

option(USE_PI_ZERO "Use RaspberryPI Zero shield as user interface" ON)
option(USE_WEBSOCKET "Use the WebSocket based user interface" ON)
option(USE_RTAUDIO "Use the RtAudio as audio backend instead of portaudio" OFF)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
  set(IS_PINK ON)
else()
  set(IS_PINK OFF)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

if(IS_PINK)
  include(colors)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU|AppleClang|Clang" )
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=256 -std=c++11 -fPIC -O3")

  if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang")
    foreach(flag_var CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
      set(${flag_var} "${${flag_var}} -fcolor-diagnostics")
    endforeach()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++")
  endif()
endif()

# Version ---------------------------------------------------------------------------------------- #

string(TIMESTAMP PINK_BUILD_DATE "%Y-%m-%d")

set (PINK_VERSION_MAJOR 0)
set (PINK_VERSION_MINOR 1)
set (PINK_VERSION_MICRO 0)
set (PINK_VERSION_STRING "${PINK_VERSION_MAJOR}.${PINK_VERSION_MINOR}.${PINK_VERSION_MICRO}")

if($ENV{APPVEYOR})
  set (PINK_BUILD_HOST "Appveyor")
  set (PINK_BUILD_NUMBER ".$ENV{APPVEYOR_BUILD_NUMBER}")
  set (PINK_GIT_BRANCH $ENV{APPVEYOR_REPO_BRANCH})
  execute_process(COMMAND appveyor UpdateBuild -Version ${PINK_VERSION_STRING})
elseif($ENV{TRAVIS})
  set (PINK_BUILD_HOST "Travis-CI")
  set (PINK_BUILD_NUMBER ".$ENV{TRAVIS_BUILD_NUMBER}")
  set (PINK_GIT_BRANCH $ENV{TRAVIS_BRANCH})
else()
  set (PINK_BUILD_HOST "a local machine")
  set (PINK_BUILD_NUMBER "")
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PINK_ROOT_DIR}
    OUTPUT_VARIABLE PINK_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(${PINK_GIT_BRANCH} STREQUAL "master")
  set(PINK_BUILD_TYPE "release version")
  set (PINK_VERSION_STRING "${PINK_VERSION_STRING}${PINK_BUILD_NUMBER}")
elseif(${PINK_GIT_BRANCH} STREQUAL "develop")
  set(PINK_BUILD_TYPE "development version")
  set (PINK_VERSION_STRING "${PINK_VERSION_STRING}${PINK_BUILD_NUMBER}-dev")
else()
  set(PINK_BUILD_TYPE "development version from feature branch: ${GIT_BRANCH}")
  set (PINK_VERSION_STRING "${PINK_VERSION_STRING}${PINK_BUILD_NUMBER}-fb")
endif()

configure_file (
  "${PINK_ROOT_DIR}/support/config/pink-config.h.in"
  "${PROJECT_BINARY_DIR}/pink-config.h"
)

if($ENV{APPVEYOR})
  execute_process(COMMAND appveyor UpdateBuild -Version ${PINK_VERSION_STRING})
  set(ENV{APPVEYOR_BUILD_VERSION} "${PINK_VERSION_STRING}")
endif()

set(PINK_BUILD_DESCRIPTION "pink v. ${PINK_VERSION_STRING} - ${PINK_BUILD_TYPE} built on ${PINK_BUILD_HOST}")

MESSAGE( STATUS "            ")
MESSAGE( STATUS "            ${Magenta}             88             ${White}  88")
MESSAGE( STATUS "            ${Magenta}             \"\"             ${White}  88")
MESSAGE( STATUS "            ${Magenta}                              ${White}88             ${ColourReset}")
MESSAGE( STATUS "            ${Magenta}8b,dPPYba,   88${White}  8b,dPPYba,   88   ,d8")
MESSAGE( STATUS "            ${Magenta}88P'    \"8a  88${White}  88P'   `\"8a  88 ,a8\"")
MESSAGE( STATUS "            ${Magenta}88       d8  88${White}  88       88  8888[")
MESSAGE( STATUS "            ${Magenta}88b,   ,a8\"  88${White}  88       88  88`\"Yba,")
MESSAGE( STATUS "            ${Magenta}88`YbbdP\"'   88${White}  88       88  88   `Y8a")
MESSAGE( STATUS "            ${Magenta}88             ${White}")
MESSAGE( STATUS "            ${Magenta}88             ${ColourReset}")
MESSAGE( STATUS "            ")

MESSAGE( STATUS "            Ableton Link for Raspberry Pi - v. ${PINK_VERSION_STRING}")
MESSAGE( STATUS "                                               ")
message( STATUS "[${PINK_BUILD_DATE}] ${PINK_BUILD_DESCRIPTION} ")

# Dependencies ----------------------------------------------------------------------------------- #

include(${PINK_MODULES_DIR}/link/AbletonLinkConfig.cmake)
add_subdirectory(${PINK_MODULES_DIR}/nmws)

set(JUST_INSTALL_CEREAL ON)
add_subdirectory(${PINK_MODULES_DIR}/cereal)


# Source files ----------------------------------------------------------------------------------- #

set(
  src_ui_SRCS
    src/ui/UserInterface.h
)

if(USE_PI_ZERO)
  message("   Raspberry PI Zero user interface enabled")
  set(
    src_ui_zero_SRCS
      src/ui/UserInterfacePiZero.cpp
      src/ui/UserInterfacePiZero.h
  )
endif()


if(USE_WEBSOCKET)
  message("   WebSocket user interface enabled")
  set(
    src_ui_ws_SRCS
      src/ui/UserInterfaceWebSocket.cpp
      src/ui/UserInterfaceWebSocket.h
  )
endif()

if(NOT(USE_WEBSOCKET) AND NOT(USE_PI_ZERO))
  set(
    src_ui_none_SRCS
      src/ui/UserInterfaceNone.h
  )
endif()

set(
  src_SRCS
    src/Audio.cpp
    src/Audio.h
    src/Engine.cpp
    src/Engine.h
    src/main.cpp
    src/Pink.cpp
    src/Pink.h
)

set(
  PINK_SRCS
    ${src_SRCS}
    ${src_ui_SRCS}
    ${src_ui_zero_SRCS}
    ${src_ui_ws_SRCS}
    ${src_ui_none_SRCS}
)

add_executable( ${PROJECT_NAME} ${PINK_SRCS} )
set_target_properties( pink PROPERTIES OUTPUT_NAME pinkd)

if(USE_PI_ZERO)
  target_compile_definitions(pink PRIVATE UI_USE_RASPBERRY_PI_ZERO)
endif()
if(USE_WEBSOCKET)
  target_link_libraries( pink PRIVATE cereal nmws)
  target_compile_definitions(pink PRIVATE UI_USE_WEBSOCKET)
endif()
if(USE_RTAUDIO)
  message("   Using RtAudio")
  add_subdirectory( ${PINK_MODULES_DIR}/rtaudio )
  target_include_directories( pink PRIVATE ${PINK_MODULES_DIR}/rtaudio )
  target_compile_definitions( pink PRIVATE AUDIO_USE_RTAUDIO )
  if(UNIX)
    target_link_libraries( pink PRIVATE rtaudio )
  else()
    target_link_libraries( pink PRIVATE rtaudio_static )
  endif()
else()
  message("   Using Portaudio")
  target_link_libraries( pink PRIVATE portaudio )
endif()

target_include_directories( pink PRIVATE src ${PINK_BUILD_DIR} /usr/include /usr/local/include)

if(APPLE)
  target_link_libraries( pink PRIVATE "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreAudio" Ableton::Link)
  target_link_libraries( pink PRIVATE /usr/local/lib/libportaudio.a)
else()
  if(USE_PI_ZERO)
    target_link_libraries( pink PRIVATE wiringPi)
  endif()

  target_link_libraries( pink PRIVATE Ableton::Link pthread)
endif()


# Install ---------------------------------------------------------------------------------------- #

if(UNIX)
  install(TARGETS pink DESTINATION /usr/sbin COMPONENT binaries)
  install(PROGRAMS ${PINK_ROOT_DIR}/support/init.d/pink DESTINATION /etc/init.d/ )
endif()
