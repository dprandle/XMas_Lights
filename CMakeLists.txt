cmake_minimum_required(VERSION 3.10.0)
project(XMAS_Lights)

set(TARGET_NAME XMAS_Lights-v1.0.0-alpha)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Multimedia REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set the source directories
set(XMAS_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(XMAS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(UI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/ui)
set(FMOD_LIB_DIR ${CMAKE_SOURCE_DIR}/deps/fmod/lib)
set(FMOD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/deps/fmod/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the src files for the project
file(GLOB SRC_FILES 
  "${XMAS_SRC_DIR}/*.cpp"
  "${XMAS_SRC_DIR}/DspFilters/*.cpp"
  )

file(GLOB UI_FILES
  "${UI_INCLUDE_DIR}/*.ui"
  )

file(GLOB HEADER_FILES
  "${XMAS_INCLUDE_DIR}/*.h"
  )

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_wrap_cpp(SRC_MOC_FILES ${HEADER_FILES})
qt5_add_resources(SRC_RES_FILE ${CMAKE_SOURCE_DIR}/icons.qrc)

# Set project includes dir
include_directories(
    ${XMAS_INCLUDE_DIR}
    ${FMOD_INCLUDE_DIR}
)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Network_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(${Qt5PrintSupport_DEFINITIONS})
add_definitions(${Qt5Multimedia_DEFINITIONS})

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
  add_definitions(-DDEBUG_VERSION)
else()
  add_definitions(-DRELEASE_VERSION)
  add_definitions(-DQT_NO_DEBUG)
endif()

if (UNIX)
  message("Adding commands")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
elseif (WIN32)
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /WX /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /std:c++14")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

if (WIN32)
    add_executable(${TARGET_NAME} WIN32 ${SRC_FILES} ${UI_HEADERS} ${SRC_RES_FILE} ${SRC_MOC_FILES} ${CMAKE_SOURCE_DIR}/XMAS_Lights.rc)
else()  
    add_executable(${TARGET_NAME} ${SRC_FILES} ${UI_HEADERS} ${SRC_RES_FILE} ${SRC_MOC_FILES})
endif()

# Create a shortcut for the compile commands
add_custom_target(move_compile_commands ALL
  COMMAND cmake -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/build/compile_commands.json
  DEPENDS ${TARGET_NAME}
  )

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)

find_library(FMOD_LIB fmod ${FMOD_LIB_DIR})
find_library(FSBANK_LIB fmod ${FMOD_LIB_DIR})


target_link_libraries(${TARGET_NAME}
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  Qt5::Network
  Qt5::PrintSupport
  Qt5::Multimedia
  ${FMOD_LIB}
  ${FSBANK_LIB}
  )