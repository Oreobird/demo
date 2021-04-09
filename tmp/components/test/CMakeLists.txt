cmake_minimum_required(VERSION 3.1)

find_package(GTest ${REQUIRED_WHEN_TESTING})

if(BRAZIL_BUILD)
	find_package(GMock ${REQUIRED_WHEN_TESTING})
else()
	set(GMOCK_INCLUDE_DIR /usr/local/include/gmock)
	set(GMOCK_LIBRARY /usr/local/lib/libgmock.a)
endif()

macro(print_includelist)
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	message(STATUS "include_dirs:")
	foreach(dir ${dirs})
  		message("    ${dir}")
	endforeach()
endmacro()

macro(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../common/include
	${CMAKE_CURRENT_SOURCE_DIR}/../os_adapter/include
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
	)

subdirlist(common_src_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../common)
foreach(dir ${common_src_dirs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/${dir})
endforeach()
	
subdirlist(hal_inc_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../hal_adapter/include)
foreach(dir ${hal_inc_dirs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../hal_adapter/include/${dir})
endforeach()
subdirlist(hal_src_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../hal_adapter/src_esp)
foreach(dir ${hal_src_dirs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../hal_adapter/src_esp/${dir})
endforeach()

subdirlist(sdk_inc_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../vesync_sdk/include)
foreach(dir ${sdk_inc_dirs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vesync_sdk/include/${dir})
endforeach()
subdirlist(sdk_src_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../vesync_sdk/src)
foreach(dir ${sdk_src_dirs})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vesync_sdk/src/${dir})
endforeach()

print_includelist()

file(GLOB_RECURSE os_adapter_src "${CMAKE_CURRENT_SOURCE_DIR}/../os_adapter/linux/*.c")
file(GLOB_RECURSE hal_adapter_src "${CMAKE_CURRENT_SOURCE_DIR}/../hal_adapter/*.c")
file(GLOB_RECURSE common_src "${CMAKE_CURRENT_SOURCE_DIR}/../common/*.c")
file(GLOB_RECURSE vesync_sdk_src "${CMAKE_CURRENT_SOURCE_DIR}/../vesync_sdk/*.c")
set(SOURCES
	${common_src}
	${os_adapter_src}
	${hal_adapter_src}
	${vesync_sdk_src}
	)

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(all_tests ${SOURCES} ${TEST_SOURCES})
target_link_libraries(
	all_tests
	${GTEST_LIBRARY} 
	${GMOCK_LIBRARY} 
	${CMAKE_THREAD_LIBS_INIT}
	gtest_main
	-lrt)

list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

foreach(file ${TEST_SOURCES})
	set(name)
	get_filename_component(name ${file} NAME_WE)
	message("${name}")
	add_executable("${name}"
		${SOURCES}
		${file})
	target_link_libraries(
		"${name}" 
		${GTEST_LIBRARY} 
		${GMOCK_LIBRARY} 
		${CMAKE_THREAD_LIBS_INIT}
		gtest_main
		-lrt)
	add_test(NAME ${name} COMMAND "${name}")
endforeach()


