cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(LogisDom LANGUAGES CXX)
elseif(UNIX)
    project(LogisDom)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo" CACHE STRING "" FORCE)

# Quand on fait du cross compiling il faut precciez les 3 lignes suivantes pour que la recherche de fichier fonctionne
set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH )


#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml SerialPort Mqtt LinguistTools OpenGL OpenGLWidgets Svg Core5Compat)

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/header")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(TRANS_DIR "${CMAKE_SOURCE_DIR}/trans")


include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})
include_directories(${TRANS_DIR})

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
    "${TRANS_DIR}/*.ts"
)



set(TS_FILES logisdom_fr.ts)

qt_add_translation(QM_FILES ${TS_FILES})



# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR} 
	${CMAKE_SOURCE_DIR}/devonewire
	${CMAKE_SOURCE_DIR}/ecogest )

file( GLOB CPP_MAIN "${CMAKE_SOURCE_DIR}/*.cpp" )

file( GLOB DEVEO_CPP "${CMAKE_SOURCE_DIR}/deveo/*.cpp" )
file( GLOB DEVEO_UI "${CMAKE_SOURCE_DIR}/deveo/*.ui" )

file( GLOB DEVONEWIRE_CPP "${CMAKE_SOURCE_DIR}/devonewire/*.cpp" )
file( GLOB DEVONEWIRE_UI "${CMAKE_SOURCE_DIR}/devonewire/*.ui" )

file( GLOB ECOGEST_CPP "${CMAKE_SOURCE_DIR}/ecogest/*.cpp" )
file( GLOB ECOGEST_UI "${CMAKE_SOURCE_DIR}/ecogest/*.ui" )

file( GLOB ENOCEAN_CPP "${CMAKE_SOURCE_DIR}/enocean/*.cpp" )
file( GLOB ENOCEAN_UI "${CMAKE_SOURCE_DIR}/enocean/*.ui" )

file( GLOB FTS800_CPP "${CMAKE_SOURCE_DIR}/fts800/*.cpp" )
file( GLOB FTS800_UI "${CMAKE_SOURCE_DIR}/fts800/*.ui" )

file( GLOB HA7NET_CPP "${CMAKE_SOURCE_DIR}/ha7net/*.cpp" )
file( GLOB HA7NET_UI "${CMAKE_SOURCE_DIR}/ha7net/*.ui" )

file( GLOB HA7S_CPP "${CMAKE_SOURCE_DIR}/ha7s/*.cpp" )
file( GLOB HA7S_UI "${CMAKE_SOURCE_DIR}/ha7s/*.ui" )

file( GLOB MAIL_CPP "${CMAKE_SOURCE_DIR}/mail/*.cpp" )
file( GLOB MAIL_UI "${CMAKE_SOURCE_DIR}/mail/*.ui" )

file( GLOB MBUS_CPP "${CMAKE_SOURCE_DIR}/mbus/*.cpp" )
file( GLOB MBUS_UI "${CMAKE_SOURCE_DIR}/mbus/*.ui" )

file( GLOB MODBUS_CPP "${CMAKE_SOURCE_DIR}/modbus/*.cpp" )
file( GLOB MODBUS_UI "${CMAKE_SOURCE_DIR}/modbus/*.ui" )

file( GLOB MQTT_CPP "${CMAKE_SOURCE_DIR}/mqtt/*.cpp" )
file( GLOB MQTT_UI "${CMAKE_SOURCE_DIR}/mqtt/*.ui" )

file( GLOB TELEINFO_CPP "${CMAKE_SOURCE_DIR}/teleinfo/*.cpp" )
file( GLOB TELEINFO_UI "${CMAKE_SOURCE_DIR}/teleinfo/*.ui" )

# Add the executable
if (WIN32) 
    add_executable(LogisDom WIN32 ${SOURCES} ${CPP_MAIN} ${DEVEO_CPP} ${DEVEO_UI} ${DEVONEWIRE_CPP} ${DEVONEWIRE_UI} ${ECOGEST_CPP} ${ECOGEST_UI} ${ENOCEAN_CPP} ${ENOCEAN_UI} ${FTS800_CPP} ${FTS800_UI} ${HA7NET_CPP} ${HA7NET_UI} ${HA7S_CPP} ${HA7S_UI} ${MAIL_CPP} ${MAIL_UI} ${MBUS_CPP} ${MBUS_UI} ${MODBUS_CPP} ${MODBUS_UI} ${MQTT_CPP} ${MQTT_UI} ${TELEINFO_CPP} ${TELEINFO_UI} )
elseif(UNIX)
    add_executable(LogisDom ${SOURCES} ${CPP_MAIN} ${DEVEO_CPP} ${DEVEO_UI} ${DEVONEWIRE_CPP} ${DEVONEWIRE_UI} ${ECOGEST_CPP} ${ECOGEST_UI} ${ENOCEAN_CPP} ${ENOCEAN_UI} ${FTS800_CPP} ${FTS800_UI} ${HA7NET_CPP} ${HA7NET_UI} ${HA7S_CPP} ${HA7S_UI} ${MAIL_CPP} ${MAIL_UI} ${MBUS_CPP} ${MBUS_UI} ${MODBUS_CPP} ${MODBUS_UI} ${MQTT_CPP} ${MQTT_UI} ${TELEINFO_CPP} ${TELEINFO_UI})
endif()

target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/deveo/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/devonewire/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/ecogest/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/enocean/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/fts800/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/ha7net/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/ha7s/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/mail/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/mbus/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/modbus/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/mqtt/*.h")
target_include_directories(LogisDom PUBLIC "${CMAKE_SOURCE_DIR}/teleinfo/*.h")


#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)


# Add the target includes for LogisDom 
target_include_directories(LogisDom PUBLIC ${FORMS_DIR})
target_include_directories(LogisDom PUBLIC ${INCLUDE_DIR})
target_include_directories(LogisDom PUBLIC ${SOURCE_DIR})
target_include_directories(LogisDom PUBLIC ${SOURCE_DIR})


set(QWT_DIR ${CMAKE_CURRENT_LIST_DIR}/../build_rpi/qwt-6.2.0)
find_path ( QWT_INCLUDE_DIR NAMES qwt_plot.h PATHS ${QWT_DIR} PATH_SUFFIXES src lib )
message( STATUS "QWT folder : ${QWT_INCLUDE_DIR}" )
include_directories( ${QWT_INCLUDE_DIR} )
find_path ( QWT_LIB_DIR NAMES libqwt.so PATHS ${QWT_DIR} PATH_SUFFIXES lib )
message( STATUS "QWT lib folder : ${QWT_LIB_DIR}" )

add_library(qwt SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(qwt PROPERTIES
  IMPORTED_LOCATION "${QWT_LIB_DIR}/libqwt.so"
  INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
)

set(QUAZIP_DIR ${CMAKE_CURRENT_LIST_DIR}/../build_rpi/quazip)
find_path ( QUAZIP_INCLUDE_DIR NAMES quazip.h PATHS ${QUAZIP_DIR} PATH_SUFFIXES quazip lib )
message( STATUS "QUAZIP folder : ${QUAZIP_INCLUDE_DIR}" )
include_directories( ${QUAZIP_INCLUDE_DIR} )
find_path ( QUAZIP_LIB_DIR NAMES libquazip1-qt6.so PATHS ${QUAZIP_DIR} PATH_SUFFIXES lib )
message( STATUS "QUAZIP lib folder : ${QUAZIP_LIB_DIR}" )

add_library(quazip1-qt6 SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(quazip1-qt6 PROPERTIES
  IMPORTED_LOCATION "${QUAZIP_LIB_DIR}/libquazip1-qt6.so"
  INTERFACE_INCLUDE_DIRECTORIES "${QUAZIP_DIR}"
)

  
#===================== LINKING LIBRARIES =======================#
target_link_libraries(LogisDom Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Xml Qt6::SerialPort Qt6::Mqtt Qt6::DBus Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg Qt6::Core5Compat qwt quazip1-qt6)


