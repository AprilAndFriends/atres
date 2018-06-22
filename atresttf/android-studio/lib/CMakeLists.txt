cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(atresttf)

include("../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(atresttf STATIC ${CppSrc})

include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")
include_directories(AFTER "../../../../april/include")
include_directories(AFTER "../../../include")
include_directories(AFTER "../../../lib/freetype/include")
include_directories(AFTER "../../include/atresttf")
include_directories(AFTER "../../src")

add_definitions(
	-DATRESTTF_EXPORTS
)

target_link_libraries(
	atresttf
)
