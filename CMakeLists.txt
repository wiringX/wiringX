cmake_minimum_required(VERSION 2.8.8)

project(wiringX C)

set(PROJECT_VERSION 1.0)
set(PROJECT_NAME wiringX)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
set(CPACK_DEB_COMPONENT_INSTALL TRUE)

#Final compilation all platforms
#Removing debugging for final compilation
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_EXE_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")
set(CMAKE_SHARED_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")
set(CMAKE_MODULE_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv6" OR CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "armhf")
		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf)
		set(CMAKE_C_FLAGS "-Ofast -mfloat-abi=hard -mfpu=vfp -march=armv6 -ffast-math -Wfloat-equal -Wshadow -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wswitch-default -Wswitch-enum -Wformat=2 -g -Wall -Wno-sign-conversion -Wunreachable-code -Wstrict-prototypes")
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7l")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf)
	set(CMAKE_C_FLAGS "-Ofast -fPIC -mfloat-abi=hard -mfpu=vfp -march=armv7-a -ffast-math -Wfloat-equal -Wshadow -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wswitch-default -Wswitch-enum -Wformat=2 -g -Wall -Wno-sign-conversion -Wunreachable-code -Wstrict-prototypes")
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^mips")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE mipsel)
	set(CMAKE_C_FLAGS "-Ofast -fPIC -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wswitch-default -Wswitch-enum -Wformat=2 -g -Wall -Wno-sign-conversion -Wunreachable-code -Wstrict-prototypes")
endif()


# Start uninstaller generator
function(WRITE_UNINSTALL_TARGET_SCRIPT)
    # Create uninstall target template file, if it doesn't exist...
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in)
        set(__uninstall_filename ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake.in)
        # BEGIN actual write to file...
        file(WRITE ${__uninstall_filename} "\# - uninstall target template\n\#")
        file(APPEND ${__uninstall_filename} "
if (NOT EXISTS \"\@CMAKE_CURRENT_BINARY_DIR\@/install_manifest.txt\")
    message(FATAL_ERROR \"Cannot find install manifest: \\\"\@CMAKE_CURRENT_BINARY_DIR\@/install_manifest.txt\\\"\")
endif(NOT EXISTS \"\@CMAKE_CURRENT_BINARY_DIR\@/install_manifest.txt\")

file(READ \"\@CMAKE_CURRENT_BINARY_DIR\@/install_manifest.txt\" files)
string(REGEX REPLACE \"\\n\" \";\" files \"\${files}\")

foreach (file \${files})
    message(STATUS \"Uninstalling \\\"\$ENV{DESTDIR}\${file}\\\"\")
    if (EXISTS \"\$ENV{DESTDIR}\${file}\")
        execute_process(
            COMMAND \@CMAKE_COMMAND\@ -E remove \"\$ENV{DESTDIR}\${file}\"
            OUTPUT_VARIABLE rm_out
            RESULT_VARIABLE rm_retval
        )
        if(NOT \${rm_retval} EQUAL 0)
            message(FATAL_ERROR \"Problem when removing \\\"\$ENV{DESTDIR}\${file}\\\"\")
        endif (NOT \${rm_retval} EQUAL 0)
    else (EXISTS \"\$ENV{DESTDIR}\${file}\")
        message(STATUS \"File \\\"\$ENV{DESTDIR}\${file}\\\" does not exist.\")
    endif (EXISTS \"\$ENV{DESTDIR}\${file}\")
endforeach(file)

execute_process(COMMAND ldconfig)

") # END of appending to file...
    endif()
endfunction()
# End of the uninstaller generator

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake_postinstall.cmake)
	set(__uninstall_filename ${CMAKE_CURRENT_BINARY_DIR}/cmake_postinstall.cmake)
endif()

execute_process(COMMAND git describe --always
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE git_result
	OUTPUT_VARIABLE git_ver)

STRING(REGEX REPLACE "\n" "" git_ver "${git_ver}")
add_definitions(-DHASH="${git_ver}")

file(GLOB headers
	"${PROJECT_SOURCE_DIR}/*.h"
)

file(GLOB sources
	"${PROJECT_SOURCE_DIR}/*.c"
)

add_library(sources OBJECT ${sources})

add_library(wiringx_static STATIC $<TARGET_OBJECTS:sources>)
add_library(wiringx_shared SHARED $<TARGET_OBJECTS:sources>)

set_target_properties(wiringx_shared wiringx_static PROPERTIES OUTPUT_NAME wiringX)

install(FILES libwiringX.so DESTINATION lib/)
install(FILES libwiringX.a DESTINATION lib/)
install(FILES wiringX.h DESTINATION include/)

WRITE_UNINSTALL_TARGET_SCRIPT()
configure_file("${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/")
set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_STRIP_FILES TRUE)
set(CPACK_PACKAGE_CONTACT "CurlyMo <info@pilight.org>")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_NAME "wiringX")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_DESCRIPTION "Control RF devices with the Raspberry Pi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Control RF devices with the Raspberry Pi")

include(CPack)
