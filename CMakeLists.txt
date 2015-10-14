
# CMakeLists.txt, generated gencmake.pl, on 2015/10/12 20:38:24
cmake_minimum_required( VERSION 2.8.8 )

# CMakeScripts or use the ones that come by default with CMake.
# set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

project( fork-and-pipe )

# The version number.
set( fp_MAJOR 3 )
set( fp_MINOR 0 )
set( fp_POINT 0 )

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Set ON to build shared Library (DLL)" OFF )

if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach(warning)
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    # to distinguish between debug and release libs
    set( CMAKE_DEBUG_POSTFIX "d" )
else()
    # items for unix
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

find_package(Threads REQUIRED)
if (Threads_FOUND)
    message(STATUS "*** Threads_FOUND CMAKE_THREAD_LIBS_INIT=${CMAKE_THREAD_LIBS_INIT}" )
    if(WIN32)
        message( STATUS "*** Finding pthread.h HINTS [${CMAKE_PREFIX_PATH}] PATHS [${CMAKE_INCLUDE_PATH}] PATH_SUFFIXES include" )
        find_path( thread_INC pthread.h
            HINTS ${CMAKE_PREFIX_PATH} $ENV{INCLUDE}
            PATHS ${CMAKE_INCLUDE_PATH} $ENV{INCLUDE}
            PATH_SUFFIXES include )
        if(thread_INC)
            include_directories( SYSTEM ${thread_INC} )
            message(STATUS "*** thread_INC: ${thread_INC}")
        else()
            message(FATAL_ERROR "*** thread_INC: ${thread_INC} pthread.h NOT FOUND!")
        endif()
        find_library( thread_LIB pthreadVC2.lib
            HINTS ${CMAKE_PREFIX_PATH} $ENV{LIB}
            PATHS ${CMAKE_INCLUDE_PATH} $ENV{LIB}
            PATH_SUFFIXES lib )
        if(thread_LIB)
            list(APPEND add_LIBS ${thread_LIB})
            message(STATUS "*** Found ${thread_LIB})" )
        else()
            message( FATAL_ERROR "*** NOT FOUND thread library" )
        endif()
    else()
        list(APPEND add_LIBS ${CMAKE_THREAD_LIBS_INIT})
    endif()
else ()
    message(FATAL_ERROR "Threads NOT FOUND1")
endif ()
# configuration file, if needed
# configure_file( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
# add_definitions( -DHAVE_CONFIG_H )
# include_directories( ${CMAKE_BINARY_DIR} )

if(BUILD_SHARED_LIB)
   set(LIB_TYPE SHARED)
   message(STATUS "*** Building DLL library ${LIB_TYPE}")
else()
   message(STATUS "*** Option BUILD_SHARED_LIB is OFF ${LIB_TYPE}")
endif()
# Total libs 0, exes 1
#################################################
##### LIBRARIES 0 #####
#################################################
#list(APPEND add_LIBS ${name})
# deal with install, if any...
#install( TARGETS ${name}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib )
#install(FILES ${${name}_HDRS} DESTINATION include)
#################################################
##### EXECUTABLE 1 #####
#################################################

# fork-and-pipe EXECUTABLE from [src],
# have 1 C/C++ sources, 0 headers
set(name fork-and-pipe)
set(dir src)
set(${name}_SRCS
    ${dir}/fork-and-pipe.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

# eof
