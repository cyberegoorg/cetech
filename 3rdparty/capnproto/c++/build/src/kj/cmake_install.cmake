# Install script for directory: /home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libkj.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/build/src/kj/libkj.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/common.h;/usr/local/include/kj/units.h;/usr/local/include/kj/memory.h;/usr/local/include/kj/refcount.h;/usr/local/include/kj/array.h;/usr/local/include/kj/vector.h;/usr/local/include/kj/string.h;/usr/local/include/kj/string-tree.h;/usr/local/include/kj/exception.h;/usr/local/include/kj/debug.h;/usr/local/include/kj/arena.h;/usr/local/include/kj/miniposix.h;/usr/local/include/kj/io.h;/usr/local/include/kj/tuple.h;/usr/local/include/kj/one-of.h;/usr/local/include/kj/function.h;/usr/local/include/kj/mutex.h;/usr/local/include/kj/thread.h;/usr/local/include/kj/threadlocal.h;/usr/local/include/kj/main.h;/usr/local/include/kj/windows-sanity.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj" TYPE FILE FILES
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/common.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/units.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/memory.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/refcount.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/array.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/vector.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/string.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/string-tree.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/exception.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/debug.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/arena.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/miniposix.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/io.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/tuple.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/one-of.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/function.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/mutex.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/thread.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/threadlocal.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/main.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/windows-sanity.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/parse/common.h;/usr/local/include/kj/parse/char.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj/parse" TYPE FILE FILES
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/parse/common.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/parse/char.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/std/iostream.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj/std" TYPE FILE FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/std/iostream.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libkj-test.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/build/src/kj/libkj-test.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/test.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj" TYPE FILE FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/test.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/compat/gtest.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj/compat" TYPE FILE FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/compat/gtest.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libkj-async.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/build/src/kj/libkj-async.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/kj/async-prelude.h;/usr/local/include/kj/async.h;/usr/local/include/kj/async-inl.h;/usr/local/include/kj/async-unix.h;/usr/local/include/kj/async-io.h;/usr/local/include/kj/time.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/kj" TYPE FILE FILES
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/async-prelude.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/async.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/async-inl.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/async-unix.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/async-io.h"
    "/home/ondra/Projects/cyberego/cetech1/3rdparty/capnproto/c++/src/kj/time.h"
    )
endif()

