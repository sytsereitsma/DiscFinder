#VS_INJECT="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
cmake_minimum_required(VERSION 3.21)

project("RFID disk finder")
 
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes
    ${PROJECT_SOURCE_DIR}/src/external
    ${PROJECT_SOURCE_DIR}/../rfid
)


SET(SOURCE
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDParser.cpp
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDParser.h
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDUtil.cpp
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDUtil.h
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDCommands.cpp
    ${PROJECT_SOURCE_DIR}/../rfid/RFIDCommands.h
    ${PROJECT_SOURCE_DIR}/../rfid/rfid.ino
)

set_source_files_properties("${PROJECT_SOURCE_DIR}/../rfid/rfid.ino" PROPERTIES LANGUAGE CXX)
if(MSVC)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/../rfid/rfid.ino PROPERTIES COMPILE_FLAGS /FI"${PROJECT_SOURCE_DIR}/src/arduino_fakes/ArduinoGlobals.h")
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(_WINUSER_) # HACK! WinUser.h (why on earth is this included?) typedefs INPUT, which Arduino also defines :-(
    
else()
    # GCC or Clang
    set_source_files_properties(${PROJECT_SOURCE_DIR}/../rfid/rfid.ino PROPERTIES COMPILE_FLAGS -include "${PROJECT_SOURCE_DIR}/src/arduino_fakes/ArduinoGlobals.h")
endif()

SET(TEST_SOURCE
    ${PROJECT_SOURCE_DIR}/src//TestRFIDParser.cpp
    ${PROJECT_SOURCE_DIR}/src/TestRFIDCommands.cpp
    ${PROJECT_SOURCE_DIR}/src/TestRFIDArduino.cpp
    ${PROJECT_SOURCE_DIR}/src/StreamOutputMatcher.h
)

SET(TEST_FAKES
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes/Stream.cpp
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes/Stream.h
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes/UART.h
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes/ArduinoGlobals.cpp
    ${PROJECT_SOURCE_DIR}/src/arduino_fakes/ArduinoGlobals.h
)

# External/downloaded sources
file(DOWNLOAD
    https://raw.githubusercontent.com/eranpeer/FakeIt/2.0.9/single_header/catch/fakeit.hpp
    ${PROJECT_SOURCE_DIR}/src/external/fakeit.hpp
)

file(DOWNLOAD
    https://github.com/catchorg/Catch2/releases/download/v2.13.7/catch.hpp
    ${PROJECT_SOURCE_DIR}/src/external/catch.hpp
)

SET(EXTERNALS
    ${PROJECT_SOURCE_DIR}/src/external/catch.hpp
    ${PROJECT_SOURCE_DIR}/src/external/fakeit.hpp
)

source_group(Source FILES ${SOURCE})
source_group(Test FILES ${TEST_SOURCE})
source_group(Test\\Fakes FILES ${TEST_FAKES})
source_group(Test\\External FILES ${EXTERNALS})

add_executable(
    rfid_test
    ${SOURCE}
    ${TEST_SOURCE}
    ${TEST_FAKES}
    ${EXTERNALS}
)
