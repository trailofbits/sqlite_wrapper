cmake_minimum_required (VERSION 3.14)
project (sqlite_wrapper)

cmake_policy(SET CMP0074 NEW)

set(SQLite3_ROOT "Location of SQLite files" CACHE PATH "SQLite 3 location")

find_package(SQLite3 REQUIRED)

add_library(sqlite_wrapper INTERFACE)
target_include_directories(sqlite_wrapper INTERFACE include/)
target_link_libraries(sqlite_wrapper INTERFACE SQLite::SQLite3)


option(BUILD_TESTS "Build sqlite_wrapper tests" ON)

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(test/)
endif()