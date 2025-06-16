cmake_minimum_required(VERSION 3.25)
project(adexml VERSION 1.0.0)

set(SOURCES
	adexml/entity.h
	adexml/errors.h
	adexml/errors.cpp
  adexml/xml_parser.h
	adexml/xml_parser.cpp
)

add_library(adexml STATIC ${SOURCES})
target_compile_features(adexml PUBLIC cxx_std_23)
target_include_directories(adexml PUBLIC ${CMAKE_CURRENT_LIST_DIR})
