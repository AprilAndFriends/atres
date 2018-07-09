cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_simple)

include("../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_simple/*.cpp"
)

add_library(demo_simple SHARED ${CppSrc})

include_directories(AFTER "../../../atresttf/include")
include_directories(AFTER "../../../../april/include")
include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")
include_directories(AFTER "../../../lib/freetype/include")

add_subdirectory("../../../atresttf/android-studio/lib" atresttf)
add_subdirectory("../../lib" atres)
add_subdirectory("../../../../april/android-studio/lib" april)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../../gtypes/android-studio/lib" gtypes)
add_subdirectory("../../../../april/lib/jpeg/android-studio/lib" jpeg)
add_subdirectory("../../../../april/lib/png/android-studio/lib" png)
add_subdirectory("../../../lib/freetype/android-studio/lib" freetype)

target_link_libraries(
	demo_simple
	atresttf
	atres
	april
	hltypes
	gtypes
	jpeg
	png
	freetype
	${SYSTEM_LIBS}
)
