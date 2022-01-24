cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(llgen)

option(BUILD_LLDUMP OFF "Build lldump")
option(BUILD_DEXPR OFF "Build dexpr")
option(BUILD_PARSER OFF "Build parsers using lex/flex and yacc/bison")
option(BUILD_STRUCTGRAM OFF "Build structgram")

find_library(MATH_LIBRARY m)

# Platform-specific defines
if(WIN32)
    add_definitions(-DWINDOWS)
    add_definitions(-DYY_NO_UNISTD_H)
elseif(UNIX)
    add_definitions(-DUNIX)
endif()

# Compiler-specific defines
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(LLGEN_SOURCES
    llgen.c
    gram2.tab.c
    scan2.yy.c
    avl3.c
    storage.c
    regexp.c
    lang.c)

set(LLDUMP_SOURCES
    lldump.c
    gram2.tab.c
    scan2.yy.c
    avl3.c
    storage.c
    regexp.c
    paramlist.c)

set(STRUCTGRAM_SOURCES
    structgram.c
    storage.c
    avl3.c)

set(DEXPR_SOURCES
    dexpr.main.c
    dexpr.c
    gregjul.c)

add_executable(llgen ${LLGEN_SOURCES})

if(BUILD_STRUCTGRAM)
    add_executable(structgram ${STRUCTGRAM_SOURCES})

    add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/structgram.c
    DEPENDS ${CMAKE_SOURCE_DIR}/structgram.ll
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
    COMMAND llgen ${CMAKE_SOURCE_DIR}/structgram.ll)
endif()

if(BUILD_DEXPR)
    add_executable(dexpr ${DEXPR_SOURCES})

    # Just unconditionally link against the m library if found
    # Does nothing on Windows and works fine on Unix-like
    if(MATH_LIBRARY)
        target_link_libraries(dexpr ${MATH_LIBRARY})
    endif()
endif()

if(BUILD_LLDUMP)
    add_executable(lldump ${LLDUMP_SOURCES})
endif()

if(BUILD_PARSERS)
    # Test for parser tools before trying to build wit them.
    find_program(LEX_EXE flex lex)
    find_program(YACC_EXE bison win-bison win_bison byacc yacc)

    if(NOT LEX_EXE)
        message(SEND_ERROR "Failed to find lex-like program.")
    endif()

    if(NOT YACC_EXE)
        message(SEND_ERROR "Failed to find yacc-like program.")
    endif()

    if(NOT LEX_EXE OR NOT YACC_EXE)
        message(FATAL_ERROR "Requested parser generation but can't find tools.")
    endif()

    # We explicitly use CMAKE_SOURCE_DIR for outputs and
    # working directory. Those are otherwise user controlled
    # and we expect them in a particular place.
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/gram2.tab.c ${CMAKE_SOURCE_DIR}/gram2.tab.h
        DEPENDS ${CMAKE_SOURCE_DIR}/gram2.y
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
        COMMAND ${YACC_EXE} -d ${CMAKE_SOURCE_DIR}/gram2.y)

    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/scan2.yy.c
        DEPENDS ${CMAKE_SOURCE_DIR}/scan2.l
        VERBATIM
        COMMAND ${LEX_EXE} -t ${CMAKE_SOURCE_DIR}/scan2.l > ${CMAKE_SOURCE_DIR}/scan2.yy.c)
endif()
