cmake_minimum_required(VERSION 3.24)

include(local_conf.cmake OPTIONAL)
include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG main
	)
FetchContent_MakeAvailable(ccommon)

version_from_git(
	LOG       OFF
	TIMESTAMP "%Y%m%d%H%M%S"
	)

project(QtaddressBundle VERSION ${VERSION} DESCRIPTION "IOTA-QT address api " LANGUAGES CXX)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

FetchContent_Declare(
        nodeConection
        GIT_REPOSITORY https://github.com/EddyTheCo/ConectionSettings.git
        GIT_TAG expose_qml_or_not
        FIND_PACKAGE_ARGS 0.4 CONFIG
        )
FetchContent_MakeAvailable(nodeConection)

FetchContent_Declare(
        account
        GIT_REPOSITORY https://github.com/EddyTheCo/account.git
        GIT_TAG expose_qml_or_not
        FIND_PACKAGE_ARGS 0.4 CONFIG
        )
FetchContent_MakeAvailable(account)

find_package(Qt6 COMPONENTS Core)
if (Qt6_FOUND)
	qt_standard_project_setup()
endif(Qt6_FOUND)

add_library(qaddr_bundle qaddr_bundle.cpp include/qaddr_bundle.hpp)

if (Qt6_FOUND)
	target_link_libraries(qaddr_bundle PUBLIC Qt6::Core)
endif(Qt6_FOUND)


target_include_directories(qaddr_bundle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qaddr_bundle>")


target_link_libraries(qaddr_bundle PUBLIC nodeConection account)


install(TARGETS qaddr_bundle
	EXPORT qaddr_bundleTargets
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qaddr_bundle
	)

install(EXPORT qaddr_bundleTargets
	FILE qaddr_bundleTargets.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qaddr_bundle
	)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfig.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qaddr_bundle
	)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfigVersion.cmake"
	VERSION ${VERSION}
	COMPATIBILITY SameMajorVersion
	)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qaddr_bundle
	)
export(EXPORT qaddr_bundleTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleTargets.cmake"
	)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(CPACK_PACKAGE_CONTACT "estervtech")
	include(CTest)
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-v${SEMVER}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}")
	if(NOT BUILD_SHARED_LIBS)
		set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-static")
	endif(NOT BUILD_SHARED_LIBS)
	if(USE_THREADS)
		set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-MThread")
	endif(USE_THREADS)
	include(CPack)
endif()
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
if(BUILD_DOCS)
	get_target_property(build_docs cmake_build_docs SOURCES)
	include(${build_docs})
endif()
