cmake_minimum_required(VERSION 3.2)

include(ExternalProject)

project(morpheus)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Eigen3)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(
	CMAKE_BUILD_TYPE
	Release
	CACHE
	STRING
	"Build type (Debug | Release | RelWithDebInfo)"
	FORCE
)

if(APPLE)
	set(
	CMAKE_CXX_COMPILER
	"/usr/bin/clang++"
	CACHE
	STRING
	"C++ Compiler"
	FORCE
	)

elseif(UNIX)
	set(
	CMAKE_CXX_COMPILER
	"/usr/bin/clang++"
	CACHE
	STRING
	"C++ Compiler"
	FORCE
	)

endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fpic -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fpic -O2 -Wall")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -fpic -O2 -Wall")

set(
	DEPS_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}/src/deps"
	CACHE
	PATH
	"Root directory for dependencies"
)

ExternalProject_Add(
	pugixml
	# TODO: Automatically download the latest version
#    DOWNLOAD_DIR ${DEPS_DIR}
#    DOWNLOAD_NAME pugixml.tar.gz
#    DOWNLOAD_COMMAND "wget https://github.com/zeux/pugixml/archive/master.zip"
	PREFIX ${DEPS_DIR}/pugixml
	SOURCE_DIR ${DEPS_DIR}/pugixml/scripts
	CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	CONFIGURE_COMMAND cmake ${DEPS_DIR}/pugixml/scripts
	BUILD_COMMAND make
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND ""
)

set(
	PUGIXML_INCLUDE_DIR
	"${DEPS_DIR}/pugixml/src"
	CACHE
	PATH
	"Pugixml headers"
)

include_directories(
	src/core
	src/core/TableModels
	src/core/Morphology
	src/core/Semantics
	${PUGIXML_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Qt5Widgets_INCLUDE_DIRS}
	${Eigen3_INCLUDE_DIRS}
)

if (APPLE)
	set(
	PUGIXML_LIB
	"${DEPS_DIR}/pugixml/scripts/libpugixml.dylib"
	CACHE
	PATH
	"Pugixml library"
	)
elseif(UNIX)
	set(
	PUGIXML_LIB
	"${DEPS_DIR}/pugixml/scripts/libpugixml.so"
	CACHE
	PATH
	"Pugixml library"
	)
endif()

set(
	CMAKE_LIBRARY_OUTPUT_DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/lib"
	CACHE
	PATH
	"Library output directory"
)

set(
	CMAKE_RUNTIME_OUTPUT_DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/bin"
	CACHE
	PATH
	"Executable output directory"
)

set(SOURCE_FILES

	src/core/Globals.hpp

	src/core/main.cpp

	src/core/MainWindow.hpp
	src/core/MainWindow.cpp

	src/core/Config.hpp
	src/core/Config.cpp

	#--------------#
	# Table models #
	#--------------#
#	src/core/TableModels/CharacterTransitionModel.hpp
	src/core/TableModels/CharacterModel.hpp
	src/core/TableModels/MorphemeModel.hpp

	#------------#
	# Morphology #
	#------------#

	src/core/Morphology/MorphemeExtractor.hpp
	src/core/Morphology/MorphemeExtractor.cpp

	src/core/Morphology/SuffixArray.hpp
	src/core/Morphology/SuffixArray.cpp

	#-------#
	# SENSE #
	#-------#

	src/core/Semantics/Sense.hpp
	src/core/Semantics/Sense.cpp

)

#resource files
set(RESOURCE_FILES

)

#.ui files
set(UI_FILES
	src/ui/MainWindow.ui
	src/ui/Config.ui
)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
#set(UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)

QT5_WRAP_CPP(MOC_HEADERS
	${HEADER_DIR}/MainWindow.hpp
	${HEADER_DIR}/Config.hpp
	${HEADER_DIR}/TableModels/CharacterModel.hpp
	${HEADER_DIR}/TableModels/MorphemeModel.hpp
	${HEADER_DIR}/Morphology/MorphemeExtractor.hpp
	${HEADER_DIR}/Morphology/SuffixArray.hpp
	${HEADER_DIR}/Semantics/Sense.hpp
)
QT5_WRAP_UI(FORM_HEADERS
	${UI_FILES}
)
QT5_ADD_RESOURCES(RESOURCES ${RESOURCE_FILES})

set(ALL_SOURCES
	${SOURCE_FILES}
	${RESOURCE_FILES}
	${UI_FILES}
)

add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${MOC_HEADERS})
add_dependencies(${PROJECT_NAME} pugixml)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${PUGIXML_LIB})
