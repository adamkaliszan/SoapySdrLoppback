cmake_minimum_required(VERSION 2.8.7)
project(SoapyLoopback CXX)

find_package(SoapySDR "0.8.0" NO_MODULE REQUIRED)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# Test for Atomics
include(CheckAtomic)
if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB OR NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
    set(ATOMIC_LIBS "atomic")
endif()

set(OTHER_LIBS "fmt" "pthread")

#enable c++11 features
if(CMAKE_COMPILER_IS_GNUCXX)

    #C++11 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wc++11-extensions")
endif(APPLE)

set (NUM_CHANNELS 1)
set (BYTES_PER_SAMPLE 2)
set (DEFAULT_BUFFER_LENGTH "(16 * 32 * 512)")
set (DEFAULT_NUM_BUFFERS 15)
set (DEFAULT_PIPE_NAME "default")

configure_file(config.h.in config.h @ONLY)

SOAPY_SDR_MODULE_UTIL(
    TARGET soapyloopback
    SOURCES
        SoapyLoopback.cpp
        SoapyLoopbackTx.cpp
        SoapyLoopbackRx.cpp
        SoapyLoopbackConnector.cpp
        Registration.cpp
        Settings.cpp
    LIBRARIES
        ${ATOMIC_LIBS}
        ${OTHER_LIBS}
)
