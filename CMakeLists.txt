cmake_minimum_required(VERSION 3.16)

include(local_conf.cmake OPTIONAL)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(cmake/VersionFromGit.cmake)

version_from_git(
	LOG       OFF
  TIMESTAMP "%Y%m%d%H%M%S"
)

project(QtaddressBundle VERSION ${VERSION} DESCRIPTION "IOTA-QT address api " LANGUAGES CXX)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/FindQt.cmake)
find_package(qclient 0 QUIET )
include(FetchContent)
if(NOT qclient_FOUND AND NOT TARGET qclient)
FetchContent_Declare(
        qclient
        GIT_REPOSITORY https://github.com/EddyTheCo/Qclient-IOTA.git
        GIT_TAG v0.0.1
        )
FetchContent_MakeAvailable(qclient)
endif(NOT qclient_FOUND AND NOT TARGET qclient)

find_package(qbech32 0 QUIET )
if(NOT qbech32_FOUND AND NOT TARGET qbech32)
FetchContent_Declare(
        qbech32
        GIT_REPOSITORY https://github.com/EddyTheCo/Qbech32.git
        GIT_TAG v0.0.1
        )
FetchContent_MakeAvailable(qbech32)
endif(NOT qbech32_FOUND AND NOT TARGET qbech32)

find_package(qed25519 0 QUIET )
if(NOT qed25519_FOUND AND NOT TARGET qed25519)
FetchContent_Declare(
        qed25519 
        GIT_REPOSITORY https://github.com/EddyTheCo/Qed25519.git
        GIT_TAG v0.0.1
        )
FetchContent_MakeAvailable(qed25519)
endif(NOT qed25519_FOUND AND NOT TARGET qed25519)

add_library(qaddr_bundle qaddr_bundle.cpp include/qaddr_bundle.hpp)

target_include_directories(qaddr_bundle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	"$<INSTALL_INTERFACE:include>")
target_link_libraries(qaddr_bundle PUBLIC Qt6::Core)

target_link_libraries(qaddr_bundle PUBLIC qclient qbech32)
target_link_libraries(qaddr_bundle PRIVATE qed25519)

install(TARGETS qaddr_bundle EXPORT qaddr_bundleTargets DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/)

install(EXPORT qaddr_bundleTargets
	FILE qaddr_bundleTargets.cmake
	DESTINATION lib/cmake/qaddr_bundle
	)
include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/qaddr_bundle"
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO
	)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfigVersion.cmake"
	VERSION ${VERSION}
	COMPATIBILITY SameMajorVersion
	)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfigVersion.cmake
	DESTINATION lib/cmake/qaddr_bundle
	)
export(EXPORT qaddr_bundleTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleTargets.cmake"
	)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
    if(NOT BUILD_SHARED_LIBS)	    
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-static")
    endif(NOT BUILD_SHARED_LIBS)		    
    include(CPack)
endif()
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
