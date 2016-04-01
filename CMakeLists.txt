cmake_minimum_required(VERSION 2.8.9)

project(wiringX C)

set(PROJECT_VERSION 1.0)
set(PROJECT_NAME wiringX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

#Final compilation all platforms
#Removing debugging for final compilation
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_EXE_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")
set(CMAKE_SHARED_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")
set(CMAKE_MODULE_LINKER_FLAGS " -Wl,-rpath=/usr/local/lib/,-rpath=/usr/lib/,-rpath=/lib/")

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
	
# The printf is used to prevent bash naming errors e.g. libwiringx-107?-0963491.deb
execute_process(COMMAND git log --oneline
	COMMAND wc -l
	COMMAND xargs printf %d
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	RESULT_VARIABLE git_result
	OUTPUT_VARIABLE git_commits)	

STRING(REGEX REPLACE "\n" "" git_ver "${git_ver}")
add_definitions(-DHASH="${git_ver}")

include_directories(${PROJECT_SOURCE_DIR}/src/)

file(GLOB wiringX
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/*.h"
	"${PROJECT_SOURCE_DIR}/src/platform/linksprite/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/linksprite/*.h"
	"${PROJECT_SOURCE_DIR}/src/platform/lemaker/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/lemaker/*.h"
	"${PROJECT_SOURCE_DIR}/src/platform/solidrun/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/solidrun/*.h"
	"${PROJECT_SOURCE_DIR}/src/platform/raspberrypi/*.c"
	"${PROJECT_SOURCE_DIR}/src/platform/raspberrypi/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/*.c"
	"${PROJECT_SOURCE_DIR}/src/soc/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/allwinner/*.c"
	"${PROJECT_SOURCE_DIR}/src/soc/allwinner/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/nxp/*.c"
	"${PROJECT_SOURCE_DIR}/src/soc/nxp/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/broadcom/*.c"
	"${PROJECT_SOURCE_DIR}/src/soc/broadcom/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/amlogic/*.h"
	"${PROJECT_SOURCE_DIR}/src/soc/amlogic/*.c"
)

add_library(sources OBJECT ${wiringX})
add_library(wiringx_static STATIC $<TARGET_OBJECTS:sources>)
add_library(wiringx_shared SHARED $<TARGET_OBJECTS:sources>)

set_target_properties(wiringx_shared wiringx_static PROPERTIES OUTPUT_NAME wiringX)

add_executable(wiringx-blink ${PROJECT_SOURCE_DIR}/examples/blink.c)
add_executable(wiringx-interrupt ${PROJECT_SOURCE_DIR}/examples/interrupt.c)
add_executable(wiringx-read ${PROJECT_SOURCE_DIR}/examples/read.c)
target_link_libraries(wiringx-blink wiringx_shared)
target_link_libraries(wiringx-interrupt wiringx_shared pthread)
target_link_libraries(wiringx-read wiringx_shared)

install(FILES ${CMAKE_BINARY_DIR}/libwiringX.so DESTINATION lib/ COMPONENT library)
install(FILES ${CMAKE_BINARY_DIR}/libwiringX.a DESTINATION lib/ COMPONENT library)
install(FILES ${PROJECT_SOURCE_DIR}/src/wiringX.h DESTINATION include/ COMPONENT library)
install(PROGRAMS ${CMAKE_BINARY_DIR}/wiringx-blink DESTINATION sbin/ COMPONENT library)
install(PROGRAMS ${CMAKE_BINARY_DIR}/wiringx-interrupt DESTINATION sbin/ COMPONENT library)
install(PROGRAMS ${CMAKE_BINARY_DIR}/wiringx-read DESTINATION sbin/ COMPONENT library)

WRITE_UNINSTALL_TARGET_SCRIPT()
configure_file("${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

set(CPACK_GENERATOR "DEB RPM")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_STRIP_FILES TRUE)
set(CPACK_PACKAGE_CONTACT "CurlyMo <info@pilight.org>")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}-${git_commits}-g${git_ver})
set(CPACK_PACKAGE_NAME "libwiringx")
set(CPACK_PACKAGE_FILE_NAME libwiringx-${git_commits}-g${git_ver})
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGE_DESCRIPTION "Cross-platform GPIO Interface")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform GPIO Interface")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${PROJECT_SOURCE_DIR}/res/deb/prerm;)

include(CPack)
