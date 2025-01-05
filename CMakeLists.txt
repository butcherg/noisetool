cmake_minimum_required(VERSION 3.19)
project(noisetool VERSION 0.1)

#set(CMAKE_CXX_STANDARD 20)


string(TIMESTAMP BUILDDATE)
add_compile_options(-ggdb -Wno-deprecated-enum-enum-conversion -DVERSION="${CMAKE_PROJECT_VERSION}"  -DBUILDDATE="${BUILDDATE}")
#add_compile_options(-DVERSION="${CMAKE_PROJECT_VERSION}"  -DBUILDDATE="${BUILDDATE}")

add_executable(noisetool src/noisetool.cpp )

find_package(PkgConfig REQUIRED)

#Establishes the target install directory for any built libraries:
set(BUILD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external/usr")


#libnoise:
if (DEFINED BUILD_LIBNOISE)
	if (DEFINED HOST)
		set(CMAKECMD "${HOST}-cmake")
	else()
		set(CMAKECMD "cmake")
	endif()
	set(CMAKE_ARGUMENTS -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=${BUILD_PREFIX} )

	include(ExternalProject)
	if (BUILD_LIBNOISE STREQUAL "GITHUB")
		message(STATUS "Configuring libnoise as an external project (GitHub clone).")
		ExternalProject_Add(libnoise_download
			PREFIX external/libnoise
			GIT_REPOSITORY	  https://github.com/eXpl0it3r/libnoise.git
			GIT_PROGRESS      ON
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (BUILD_LIBNOISE STREQUAL "SRCPKG")
		message(STATUS "Configuring libnoise as an external project (URL download).")
		ExternalProject_Add(libnoise_download
			PREFIX external/libnoise
			#DOWNLOAD_DIR "external"
			URL https://github.com/eXpl0it3r/libnoise/archive/refs/heads/master.zip
			DOWNLOAD_EXTRACT_TIMESTAMP FALSE
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	elseif (EXISTS ${BUILD_LIBNOISE})
		message(STATUS "Configuring libnoise as an external project (local file: ${BUILD_LIBNOISE}).")
		ExternalProject_Add(libnoise_download
			URL ${BUILD_LIBNOISE}
			DOWNLOAD_EXTRACT_TIMESTAMP FALSE
			PREFIX external/libnoise
			CMAKE_COMMAND	${CMAKECMD}
			CMAKE_ARGS ${CMAKE_ARGUMENTS}
			INSTALL_COMMAND make install
			UPDATE_COMMAND ""
		)
	else ()
		message(FATAL_ERROR "BUILD_LIBNOISE not valid: ${BUILD_LIBNOISE} (Either GITHUB, SRCPKG, or valid local path/srcpackage")
	endif()

	set(LIBNOISE_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/include)
	set(LIBNOISE_INC_DIR ${LIBNOISE_INCLUDE_DIR} CACHE STRING "libnoise include directory")
	set(LIBNOISE_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib)
	set(LIBNOISE_LIB ${CMAKE_CURRENT_BINARY_DIR}/external/usr/lib/libnoise.a)
	
	add_library(libnoise STATIC IMPORTED)
	set_target_properties(libnoise PROPERTIES IMPORTED_LOCATION ${LIBNOISE_LIB_DIR}/libnoise.a)
	set(LIBNOISE_FOUND TRUE)
	add_dependencies(noisetool libnoise_download)
	
	target_include_directories(noisetool PRIVATE ${LIBNOISE_INCLUDE_DIR})
	target_link_libraries(noisetool ${LIBNOISE_LIB}  ${LIBNOISE_DEPS})
else()
	pkg_check_modules(LIBNOISE REQUIRED libnoise)
	if (LIBNOISE_FOUND)
		set(LIBNOISE_INC_DIR ${LIBNOISE_INCLUDE_DIR} CACHE STRING "threepp include directory")
		include_directories(${LIBNOISE_INCLUDE_DIR})
		target_link_libraries(noisetool ${LIBNOISE_LIBRARIES})
	else()
		message(FATAL_ERROR "libnoise not found")
	endif(LIBNOISE_FOUND)
endif ()


if(WIN32)
#	target_link_libraries(noisetool wsock32 ws2_32 psapi)
endif(WIN32)

add_subdirectory(src)

