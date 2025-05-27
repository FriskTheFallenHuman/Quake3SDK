cmake_minimum_required(VERSION 3.20)
project(Quake3 CXX)

option(LINUX_RELEASE_BINS "Set RPATH to \$ORIGIN/libs/ for Linux binary releases" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sys/cmake")

if(LINUX_RELEASE_BINS)
	message(STATUS "Setting RPATH to \$ORIGIN/libs/ so you can put dependencies in there")
	set(CMAKE_SKIP_RPATH OFF CACHE BOOL "Skip RPATH" FORCE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/libs")
	set(ldflags "${ldflags} -Wl,-z,origin") # not sure this is even needed, but shouldn't hurt
else()
	set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
endif()

include(CheckCXXCompilerFlag)
include(GNUInstallDirs OPTIONAL RESULT_VARIABLE GNUINSTALLDIRS)

option(MISSIONPACK	"Build the team arena expansion pack" OFF)
option(DEDICATED	"Build the dedicated server" ON)
option(ONATIVE		"Optimize for the host CPU" OFF)
option(HARDLINK_GAME "Compile gamecode into executable (no game DLLs)" OFF)

if(NOT MSVC) # GCC/clang or compatible.
	option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored compiler warnings/errors (GCC/Clang only; esp. useful with ninja)." OFF)
	option(ASAN		"Enable GCC/Clang Adress Sanitizer (ASan)" OFF)
	option(UBSAN	"Enable GCC/Clang Undefined Behavior Sanitizer (UBSan), implies HARDLINK_GAME" OFF)

	if(UBSAN AND NOT HARDLINK_GAME)
		message(STATUS "UBSAN requires linking the gamecode into the executable, will enable HARDLINK_GAME")
		set(HARDLINK_GAME ON)
	endif()
else()
	option(ASAN		"Enable Visual Studio Adress Sanitizer (ASan)" OFF)
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR)
	message(FATAL_ERROR "No target CPU architecture set")
endif()

if(NOT CMAKE_SYSTEM_NAME)
	message(FATAL_ERROR "No target OS set")
endif()

# target cpu
set(cpu ${CMAKE_SYSTEM_PROCESSOR})

# Originally, ${CMAKE_SYSTEM_PROCESSOR} was supposed to contain the *target* CPU, according to CMake's documentation.
# As far as I can tell this has always been broken (always returns host CPU) at least on Windows
# (see e.g. https://cmake.org/pipermail/cmake-developers/2014-September/011405.html) and wasn't reliable on
# other systems either, for example on Linux with 32bit userland but 64bit kernel it returned the kernel CPU type
# (e.g. x86_64 instead of i686). Instead of fixing this, CMake eventually updated their documentation in 3.20,
# now it's officially the same as CMAKE_HOST_SYSTEM_PROCESSOR except when cross-compiling (where it's explicitly set)
# So we gotta figure out the actual target CPU type ourselves.. (why am I sticking to this garbage buildsystem?)
if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR))
	# special case: cross-compiling, here CMAKE_SYSTEM_PROCESSOR should be correct, hopefully
	# (just leave cpu at ${CMAKE_SYSTEM_PROCESSOR})
elseif(MSVC)
	# because all this wasn't ugly enough, it turned out that, unlike standalone CMake, Visual Studio's
	# integrated CMake doesn't set CMAKE_GENERATOR_PLATFORM, so I gave up on guessing the CPU arch here
	# and moved the CPU detection to MSVC-specific code in neo/sys/platform.h
else() # not MSVC and not cross-compiling, assume GCC or clang (-compatible), seems to work for MinGW as well
	execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpmachine"
	                RESULT_VARIABLE cc_dumpmachine_res
	                OUTPUT_VARIABLE cc_dumpmachine_out)
	if(cc_dumpmachine_res EQUAL 0)
		string(STRIP ${cc_dumpmachine_out} cc_dumpmachine_out) # get rid of trailing newline
		message(STATUS "`${CMAKE_C_COMPILER} -dumpmachine` says: \"${cc_dumpmachine_out}\"")
		# gcc -dumpmachine and clang -dumpmachine seem to print something like "x86_64-linux-gnu" (gcc)
		# or "x64_64-pc-linux-gnu" (clang) or "i686-w64-mingw32" (32bit mingw-w64) i.e. starting with the CPU,
		# then "-" and then OS or whatever - so use everything up to first "-"
		string(REGEX MATCH "^[^-]+" cpu ${cc_dumpmachine_out})
		message(STATUS "  => CPU architecture extracted from that: \"${cpu}\"")
	else()
		message(WARNING "${CMAKE_C_COMPILER} -dumpmachine failed with error (code) ${cc_dumpmachine_res}")
		message(WARNING "will use the (sometimes incorrect) CMAKE_SYSTEM_PROCESSOR (${cpu}) to determine CPU_STRING")
	endif()
endif()

if(cpu STREQUAL "powerpc")
	set(cpu "ppc")
elseif(cpu STREQUAL "aarch64")
	# "arm64" is more obvious, and some operating systems (like macOS) use it instead of "aarch64"
	set(cpu "arm64")
elseif(cpu MATCHES "i.86")
	set(cpu "x86")
elseif(cpu MATCHES "[aA][mM][dD]64" OR cpu MATCHES "[xX]64")
	set(cpu "x86_64")
elseif(cpu MATCHES "[aA][rR][mM].*") # some kind of arm..
	# On 32bit Raspbian gcc -dumpmachine returns sth starting with "arm-",
	# while clang -dumpmachine says "arm6k-..." - try to unify that to "arm"
	if(CMAKE_SIZEOF_VOID_P EQUAL 8) # sizeof(void*) == 8 => must be arm64
		set(cpu "arm64")
	else() # should be 32bit arm then (probably "armv7l" "armv6k" or sth like that)
		set(cpu "arm")
	endif()
endif()

# target os
if(APPLE)
	set(os "macosx")
else()
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" os)
endif()

add_definitions(-DOSTYPE="${os}" -DSIZEOFPTR=${CMAKE_SIZEOF_VOID_P})

if(MSVC)
	# for MSVC CPU_STRING is set in code (in code/qcommon/q_shared.h)
	message(STATUS "Setting -DSIZEOFPTR=${CMAKE_SIZEOF_VOID_P} -DOSTYPE=\"${os}\" - NOT setting CPU_STRING, because we're targeting MSVC (VisualC++)")
	# make sure ${cpu} isn't (cant't be) used by CMake when building with MSVC
	unset(cpu)
else()
	add_definitions(-DCPU_STRING="${cpu}")
	message(STATUS "Setting -DCPU_STRING=\"${cpu}\" -DSIZEOFPTR=${CMAKE_SIZEOF_VOID_P} -DOSTYPE=\"${os}\" ")

	if(cpu MATCHES ".*64.*" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		# tough luck if some CPU architecture has "64" in its name but uses 32bit pointers
		message(SEND_ERROR "CMake thinks sizeof(void*) == 4, but the target CPU ${cpu} looks like a 64bit CPU!")
		message(FATAL_ERROR "If you're building in a 32bit chroot on a 64bit host, switch to it with 'linux32 chroot' or at least call cmake with linux32 (or your OSs equivalent)!")
	endif()
endif()

if(CMAKE_CONFIGURATION_TYPES) 
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 
endif()

# build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(HARDLINK_GAME)
	add_definitions(-DQ3_STATIC)
endif()

if(MISSIONPACK)
	add_definitions(-DMISSIONPACK)
endif()

set(OUTPUT_FOLDER ${CMAKE_SOURCE_DIR}/../build/${os})

set(COMPILER_IS_CLANG FALSE)
set(COMPILER_IS_GCC_OR_CLANG FALSE)

if(NOT MSVC)
	# check if this is some kind of clang (Clang, AppleClang, whatever)
	# (convert compiler ID to lowercase so we match Clang, clang, AppleClang etc, regardless of case)
	string(TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler_id_lower)
	if(compiler_id_lower MATCHES ".*clang.*")
		message(STATUS "Compiler \"${CMAKE_CXX_COMPILER_ID}\" detected as some kind of clang")
		set(COMPILER_IS_CLANG TRUE)
		set(COMPILER_IS_GCC_OR_CLANG TRUE)
	elseif(CMAKE_COMPILER_IS_GNUCC)
		set(COMPILER_IS_GCC_OR_CLANG TRUE)
	endif()
	unset(compiler_id_lower)
endif() # not MSVC

# compiler specific flags
if(COMPILER_IS_GCC_OR_CLANG)
	add_compile_options(-pipe)
	add_compile_options(-Wall)

	if(NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_compile_options(-march=native)
	elseif(NOT APPLE AND cpu STREQUAL "x86")
		add_compile_options(-march=pentium3)
	endif()

	if(FORCE_COLORED_OUTPUT)
		if(CMAKE_COMPILER_IS_GNUCC)
		   add_compile_options (-fdiagnostics-color=always)
		elseif (COMPILER_IS_CLANG)
		   add_compile_options (-fcolor-diagnostics)
		endif ()
	endif ()

	if(cpu STREQUAL "e2k" AND CMAKE_COMPILER_IS_GNUCC)
		# O3 on E2K mcst-lcc approximately equal to O2 at X86/ARM gcc
		set(OPT_LEVEL "-O3")
	else()
		set(OPT_LEVEL "-O2")
	endif()

	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -D_DEBUG -O0")
	set(CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG")
	set(CMAKE_C_FLAGS_PROFILE "-g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELEASE "${OPT_LEVEL} -fno-math-errno -fno-trapping-math  -ffinite-math-only -fomit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -ggdb ${OPT_LEVEL} -fno-math-errno -fno-trapping-math  -ffinite-math-only -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_MINSIZEREL "-Os -fno-math-errno -fno-trapping-math  -ffinite-math-only -fomit-frame-pointer")

	set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})

	add_compile_options(-fno-strict-aliasing)
	# dear idiot compilers, don't fuck up math code with useless FMA "optimizations"
	# (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100839)
	CHECK_CXX_COMPILER_FLAG("-ffp-contract=off" cxx_has_fp-contract)
	if(cxx_has_fp-contract)
		add_compile_options(-ffp-contract=off)
	endif()
	
	if(ASAN)
		# if this doesn't work, ASan might not be available on your platform, don't set ASAN then..
		add_compile_options(-fsanitize=address)
		set(SYS_LIBS ${SYS_LIBS} -fsanitize=address)
	endif()
	if(UBSAN)
		# if this doesn't work, UBSan might not be available on your platform, don't set UBSAN then..
		add_compile_options(-fsanitize=undefined)
		set(SYS_LIBS ${SYS_LIBS} -fsanitize=undefined)
	endif()

	if(NOT AROS)
		CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" cxx_has_fvisibility)
		if(NOT cxx_has_fvisibility)
			message(FATAL_ERROR "Compiler does not support -fvisibility")
		endif()
		add_compile_options(-fvisibility=hidden)
	endif()

	# TODO fix these warnings
	add_compile_options(-Wno-sign-compare)
	add_compile_options(-Wno-switch)

	CHECK_CXX_COMPILER_FLAG("-Woverloaded-virtual" cxx_has_Woverload_virtual)
	if(cxx_has_Woverload_virtual)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
	endif()
	
	# shut up about using memcpy() on classes, in the cases doom3 uses it it seems to be fine
	CHECK_CXX_COMPILER_FLAG("-Wno-class-memaccess" cxx_has_Wno-class-memaccess)
	if(cxx_has_Wno-class-memaccess)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
	endif()

	# ignore warnings about variables named "requires" for now (in C++20 it's a keyword,
	#  but currently we don't even use C++11 features)
	CHECK_CXX_COMPILER_FLAG("-Wno-c++20-compat" cxx_has_Wno-cpp20-compat)
	if(cxx_has_Wno-cpp20-compat)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++20-compat")
	endif()

	if(AROS)
		set(CMAKE_SHARED_LIBRARY_SUFFIX ".aros-${cpu}")
		add_definitions(-DIOAPI_NO_64)
	elseif(APPLE)
		add_definitions(-DMACOS_X=1)

		if(cpu STREQUAL "x86_64")
			add_compile_options(-arch x86_64 -mmacosx-version-min=10.7)
			set(ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.7")
		elseif(cpu STREQUAL "arm64")
			add_compile_options(-arch arm64 -mmacosx-version-min=10.7)
			set(ldflags "${ldflags} -arch arm64 -mmacosx-version-min=10.7")
		elseif(cpu STREQUAL "x86")
			CHECK_CXX_COMPILER_FLAG("-arch i386" cxx_has_arch_i386)
			if(cxx_has_arch_i386)
				add_compile_options(-arch i386)
				set(ldflags "${ldflags} -arch i386")
			endif()

			add_compile_options(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		elseif(cpu STREQUAL "ppc")
			CHECK_CXX_COMPILER_FLAG("-arch ppc" cxx_has_arch_ppc)
			if(cxx_has_arch_ppc)
				add_compile_options(-arch ppc -mone-byte-bool)
				set(ldflags "${ldflags} -arch ppc -mone-byte-bool")
			endif()

			add_compile_options(-mmacosx-version-min=10.4)
			set(ldflags "${ldflags} -mmacosx-version-min=10.4")
		else()
			message(FATAL_ERROR "Unsupported CPU architecture for OSX")
		endif()

		set(SYS_LIBS ${SYS_LIBS} "-framework Carbon -framework Cocoa -framework IOKit")
	elseif(WIN32)
		# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
		set(ldflags "${ldflags} -static-libgcc -static-libstdc++ -Wl,--stack,8388608")
	elseif(os STREQUAL "linux")
		set(SYS_LIBS ${SYS_LIBS} dl)
	elseif(os STREQUAL "freebsd")
		find_library(EXECINFO_LIBRARIES execinfo /usr/lib /usr/local/lib)
		set(SYS_LIBS ${SYS_LIBS} ${EXECINFO_LIBRARIES})
	endif()
elseif(MSVC)
	add_compile_options(/MP) # parallel build (use all cores, or as many as configured in VS)
	
	add_compile_options(/W3) # TODO: was /W4, caused trouble with VS2019 (and/or its integrated CMake? or only HarrieVG's setup?)
	add_compile_options(/we4840) # treat as error when passing a class to a vararg-function (probably printf-like)
	# treat several kinds of truncating int<->pointer conversions as errors (for more 64bit-safety)
	add_compile_options(/we4306 /we4311 /we4312 /we4302)
	# ignore the following warnings:
	add_compile_options(/wd4996)   # 'function': was declared deprecated
	add_compile_options(/wd4018)   # signed/unsigned mismatch
	add_compile_options(/wd4032)
	add_compile_options(/wd4051)
	add_compile_options(/wd4057)   # slightly different base types
	add_compile_options(/wd4100)   # unreferenced formal parameter
	add_compile_options(/wd4115)
	add_compile_options(/wd4125)   # decimal digit terminates octal escape sequence
	add_compile_options(/wd4127)   # conditional expression is constant
	add_compile_options(/wd4136)
	add_compile_options(/wd4152)   # nonstandard extension, function/data pointer conversion in expression
	add_compile_options(/wd4244)   # possible loss of data
	add_compile_options(/wd4142)   # benign redefinition
	add_compile_options(/wd4514)
	add_compile_options(/wd4702)   # unreachable code
	add_compile_options(/wd4711)   # selected for automatic inline expansion
	add_compile_options(/wd4220)   # varargs matches remaining parameters

	set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd")
	set(CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD")
	set(CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD")
	# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
	set(ldflags "${ldflags} /STACK:8388608")
else()
	message(FATAL_ERROR "Unsupported compiler")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
# disable assertions in release builds
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG ${CMAKE_C_FLAGS_MINSIZEREL}")

# mingw and msvc
if(WIN32)
	add_definitions(-DWINVER=0x0501)
	add_definitions(-D_WIN32_WINNT=0x0501)

	set(SYS_LIBS ${SYS_LIBS}
		winmm
		wsock32
		dsound
		odbc32
		odbccp32
		opengl32)
endif()

include_directories(${CMAKE_SOURCE_DIR})

set(bindir		"${CMAKE_INSTALL_FULL_BINDIR}")
set(libdir		"${CMAKE_INSTALL_FULL_LIBDIR}/${PROJEC_NAME}")
set(datadir		"${CMAKE_INSTALL_FULL_DATADIR}/${PROJEC_NAME}")

if(NOT MSVC)
	message(STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${cpu}")
endif()

if(NOT APPLE AND NOT WIN32)
	message(STATUS "The install target will use the following directories:")
	message(STATUS "  Binary directory:  ${bindir}")
	message(STATUS "  Library directory: ${libdir}")
	message(STATUS "  Data directory:    ${datadir}")
endif()

set(ENGINE_DEPENDENCIES)

add_subdirectory(cgame)
add_subdirectory(game)
add_subdirectory(ui)

# Client
file(GLOB CLIENT_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp)
source_group( "Client" FILES ${CLIENT_SOURCES} )
file(GLOB CLIENT_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/client/*.h)
source_group( "Client" FILES ${CLIENT_HEADERS} )

# CM
file(GLOB CM_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/cm/*.cpp)
source_group( "CM" FILES ${CM_SOURCES} )
file(GLOB CM_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/cm/*.h)
source_group( "CM" FILES ${CM_HEADERS} )

# QCommon (Engine)
file(GLOB QCOMMON_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/qcommon/*.cpp)
list(REMOVE_ITEM QCOMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_shared.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_libvar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_memory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_precomp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_script.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_struct.cpp)
source_group( "Common" FILES ${QCOMMON_SOURCES} )
file(GLOB QCOMMON_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/qcommon/*.h)
list(REMOVE_ITEM QCOMMON_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_shared.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_libvar.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_memory.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_precomp.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_script.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_struct.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_utils.h)
source_group( "Common" FILES ${QCOMMON_HEADERS} )

# Renderer
file(GLOB RENDERER_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/renderer/*.cpp)
source_group( "Renderer" FILES ${RENDERER_SOURCES} )
file(GLOB RENDERER_GLEW_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/renderer/glew-2.2.0/include/GL/*.h)
source_group( "Renderer//Glew" FILES ${RENDERER_GLEW_HEADERS} )
file(GLOB RENDERER_STB_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/renderer/stb/*.h)
source_group( "Renderer//Stb" FILES ${RENDERER_STB_HEADERS} )
file(GLOB RENDERER_TINYJPEG_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/renderer/tinyjpeg/*.h)
source_group( "Renderer//TinyJPEG" FILES ${RENDERER_TINYJPEG_HEADERS} )
file(GLOB RENDERER_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/renderer/*.h)
source_group( "Renderer" FILES ${RENDERER_HEADERS} )

# Server
file(GLOB SERVER_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/server/*.cpp)
list(REMOVE_ITEM SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/server/sv_rankings.cpp )
source_group( "Server" FILES ${SERVER_SOURCES} )
file(GLOB SERVER_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/server/*.h)
source_group( "Server" FILES ${SERVER_HEADERS} )

# Sound
file(GLOB SOUND_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/sound/*.cpp)
source_group( "Sound" FILES ${SOUND_SOURCES} )
file(GLOB SOUND_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/sound/*.h)
source_group( "Sound" FILES ${SOUND_HEADERS} )

# Sys
if(WIN32)
	file(GLOB SYS_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/win32/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/win32/*.rc
		${CMAKE_CURRENT_SOURCE_DIR}/win32/*.ico)
	source_group( "Win32" FILES ${SYS_SOURCES} )
	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/win32/*.h)
	source_group( "Win32" FILES ${SYS_HEADERS} )
elseif(UNIX)
	file(GLOB SYS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.cpp)
	source_group( "Unix" FILES ${SYS_SOURCES} )
	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.h)
	source_group( "Unix" FILES ${SYS_HEADERS} )
elseif(APPLE)
	file(GLOB SYS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.cpp)
	source_group( "MacOSX" FILES ${SYS_SOURCES} )
	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.h)
	source_group( "MacOSX" FILES ${SYS_HEADERS} )
endif()

# Sys (Dedicated)
if(WIN32)
	file(GLOB SYS_DEDICATED_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/win32/resource.h ${CMAKE_CURRENT_SOURCE_DIR}/win32/*.rc ${CMAKE_CURRENT_SOURCE_DIR}/win32/*.ico)
	source_group( "Win32" FILES ${SYS_DEDICATED_RESOURCES} )
	set(SYS_DEDICATED_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/win32/win_main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/win32/win_net.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/win32/win_shared.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/win32/win_syscon.cpp)
	source_group( "Win32" FILES ${SYS_DEDICATED_SOURCES} )
	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/win32/*.h)
	set(SYS_DEDICATED_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/win32/win_local.h)
	source_group( "Win32" FILES ${SYS_DEDICATED_HEADERS} )
#elseif(UNIX)
#	file(GLOB SYS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.cpp)
#	source_group( "Unix" FILES ${SYS_SOURCES} )
#	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.h)
#	source_group( "Unix" FILES ${SYS_HEADERS} )
#elseif(APPLE)
#	file(GLOB SYS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.cpp)
#	source_group( "MacOSX" FILES ${SYS_SOURCES} )
#	file(GLOB SYS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/unix/*.h)
#	source_group( "MacOSX" FILES ${SYS_HEADERS} )
endif()

# Qcommon (Library)
set(QCOMMON_LIB_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_shared.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_libvar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_memory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_precomp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_script.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_struct.cpp)
source_group( "Common" FILES ${QCOMMON_LIB_SOURCES} )
set(QCOMMON_LIB_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/q_shared.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_libvar.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_memory.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_precomp.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_script.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_struct.h
	${CMAKE_CURRENT_SOURCE_DIR}/qcommon/l_utils.h)
source_group( "Common" FILES ${QCOMMON_LIB_HEADERS} )
add_library(qcommon STATIC ${QCOMMON_LIB_SOURCES} ${QCOMMON_LIB_HEADERS})

set(ENGINE_SOURCES
	${CLIENT_SOURCES}
	${CLIENT_HEADERS}
	${CM_SOURCES}
	${CM_HEADERS}
	${QCOMMON_SOURCES}
	${QCOMMON_HEADERS}
	${RENDERER_SOURCES}
	${RENDERER_GLEW_HEADERS}
	${RENDERER_STB_HEADERS}
	${RENDERER_TINYJPEG_HEADERS}
	${RENDERER_HEADERS}
	${SERVER_SOURCES}
	${SERVER_HEADERS}
	${SOUND_SOURCES}
	${SOUND_HEADERS}
	${SYS_SOURCES}
	${SYS_HEADERS})

set(DEDICATED_SOURCES
	${CM_SOURCES}
	${CM_HEADERS}
	${QCOMMON_SOURCES}
	${QCOMMON_HEADERS}
	${SERVER_SOURCES}
	${SERVER_HEADERS}
	${SYS_DEDICATED_SOURCES}
	${SYS_DEDICATED_RESOURCES}
	${SYS_DEDICATED_HEADERS})

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${ENGINE_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/renderer/glew-2.2.0/lib/Win32)
if(HARDLINK_GAME)
	add_dependencies(${PROJECT_NAME} qcommon cgame game ui)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SYS_LIBS} glew32 qcommon cgame game ui)
else()
	add_dependencies(${PROJECT_NAME} qcommon)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SYS_LIBS} glew32 qcommon )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER})
if(MSVC)
	# set MSVC default debug directory on executable
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_COMMAND "${OUTPUT_FOLDER}/${PROJECT_NAME}.exe")
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_FOLDER}")
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "+set r_fullscreen 0 +set developer 1")
endif()

if (DEDICATED)
	add_executable(${PROJECT_NAME}ded WIN32 MACOSX_BUNDLE ${DEDICATED_SOURCES})
	target_compile_definitions(${PROJECT_NAME}ded PRIVATE DEDICATED)
	if(HARDLINK_GAME)
		add_dependencies(${PROJECT_NAME}ded qcommon game)
		target_link_libraries(${PROJECT_NAME}ded PRIVATE ${SYS_LIBS} qcommon game)
	else()
		add_dependencies(${PROJECT_NAME}ded qcommon)
		target_link_libraries(${PROJECT_NAME}ded PRIVATE ${SYS_LIBS} qcommon)
	endif()
	set_target_properties(${PROJECT_NAME}ded PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER})
endif()