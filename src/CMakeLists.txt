set(FXLOAD_SOURCES
    ezusb.h
	ezusb.c
	main.cpp
	ApplicationPaths.cpp
	ApplicationPaths.h
	fxload-version.h
	${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

# Set up version file
configure_file(fxload-version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/fxload-version.cpp)

add_executable(fxload ${FXLOAD_SOURCES})
target_link_libraries(fxload libusb1::libusb1 CLI11)
target_include_directories(fxload PRIVATE .)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# On Windows we need Shlwapi.lib for PathRemoveFileSpecA
	target_link_libraries(fxload Shlwapi)
endif()

# On Windows, we also want to install any runtime dependencies needed by the executable,
# other than the Microsoft UCRT.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# This block from https://stackoverflow.com/a/72088102/7083698
	set(RUNTIME_DEPENDENCIES_CLAUSE RUNTIME_DEPENDENCIES
		PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "hvsifiletrust" "C:/msys64/mingw64/bin.*" ".*/msys64/mingw64/bin.*"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" ".*/msys64/mingw64/bin.*"
		DIRECTORIES $ENV{PATH}
	)
else()
	set(RUNTIME_DEPENDENCIES_CLAUSE "")
endif()


install(TARGETS fxload DESTINATION bin ${RUNTIME_DEPENDENCIES_CLAUSE})