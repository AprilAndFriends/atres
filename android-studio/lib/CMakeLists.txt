cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(atres)

include("../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(atres STATIC ${CppSrc})

include_directories(AFTER "../../include/atres")
include_directories(AFTER "../../src")
include_directories(AFTER "../../../april/include")
include_directories(AFTER "../../../hltypes/include")
include_directories(AFTER "../../../gtypes/include")
include_directories(AFTER "../../lib/jpeg")
include_directories(AFTER "../../lib/png")
include_directories(AFTER "../../../hltypes/lib/zlib")

add_definitions(
	-DATRES_EXPORTS
)

target_link_libraries(
	atres
)
