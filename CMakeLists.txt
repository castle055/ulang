cmake_minimum_required(VERSION 3.26)
project(ulang)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 23)

include(CheckCXXCompilerFlag)
function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if (flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if (flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif ()
        unset(flag_supported CACHE)
    endif ()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-Werror")
enable_cxx_compiler_flag_if_supported("-Wno-unused-function")
enable_cxx_compiler_flag_if_supported("-Wno-unused-parameter")
enable_cxx_compiler_flag_if_supported("-Wno-unused-variable")
enable_cxx_compiler_flag_if_supported("-Wno-unused-value")
enable_cxx_compiler_flag_if_supported("-Wno-unused-but-set-variable")
enable_cxx_compiler_flag_if_supported("-Wno-missing-field-initializers")
enable_cxx_compiler_flag_if_supported("-Wno-unknown-pragmas")
enable_cxx_compiler_flag_if_supported("-Wno-maybe-uninitialized")

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(/usr/include/freetype2)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/glib-2.0/include)

FILE(GLOB_RECURSE SRC_LIST
        ${CMAKE_CURRENT_LIST_DIR}/include/*.c
        ${CMAKE_CURRENT_LIST_DIR}/include/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/include/*.h
        ${CMAKE_CURRENT_LIST_DIR}/include/*.hh
        ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/*.c
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/*.h
        ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp
)

find_package(ECL REQUIRED)
include_directories(${ECL_INCLUDE_DIR})

LIST(APPEND EXT_LIBS cydstd cyd_ui cyd_ui_std yaml-cpp playerctl glib-2.0 gobject-2.0 X11 Xft Xrender fontconfig jpeg ${ECL_LIBRARIES})

add_executable(ulang ${SRC_LIST})
target_link_libraries(ulang PUBLIC ${EXT_LIBS})

FILE(GLOB_RECURSE TEST_LIST
        ${CMAKE_CURRENT_LIST_DIR}/tests/*.test.cpp)
FOREACH(ENTRYPOINT IN LISTS TEST_LIST)
    get_filename_component(ENTRYPOINT_NAME ${ENTRYPOINT} NAME_WE)
    set(ENTRYPOINT_NAME TEST--${ENTRYPOINT_NAME})
    add_executable(${ENTRYPOINT_NAME} ${SRC_LIST} ${ENTRYPOINT} )
    target_link_libraries(${ENTRYPOINT_NAME} PUBLIC ${EXT_LIBS})
ENDFOREACH()

