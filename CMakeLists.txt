cmake_minimum_required(VERSION 3.24)

option(USE_QML "Create QML types and modules" ON)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)

include(FetchContent)
FetchContent_Declare(
	ccommon
	GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
	GIT_TAG v0.3.0
	)
FetchContent_MakeAvailable(ccommon)

version_from_git(
	LOG       OFF
	TIMESTAMP "%Y%m%d%H%M%S"
	)

project(IotaAddressWallet VERSION ${VERSION} DESCRIPTION "Address wallet for Iota " LANGUAGES CXX)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GNUInstallDirs)

find_package(OpenSSL) #Without this qvault cant find openssl in windows and Macos :)

FetchContent_Declare(
	IotaNodeConn
	GIT_REPOSITORY https://github.com/EddyTheCo/ConectionSettings.git
	GIT_TAG v0.6.0
	FIND_PACKAGE_ARGS 0.6 CONFIG
	)
FetchContent_MakeAvailable(IotaNodeConn)

FetchContent_Declare(
	IotaAccount
	GIT_REPOSITORY https://github.com/EddyTheCo/account.git
	GIT_TAG v0.7.0
	FIND_PACKAGE_ARGS 0.7 CONFIG
	)
FetchContent_MakeAvailable(IotaAccount)

if(USE_QML)
	find_package(Qt6 COMPONENTS Core Gui Quick)
else()
	find_package(Qt6 COMPONENTS Core)
endif(USE_QML)

if (Qt6_FOUND)
	qt_standard_project_setup()

	add_library(addrBundle qaddr_bundle.cpp include/qaddr_bundle.hpp)
	add_library(${PROJECT_NAME}::addrBundle ALIAS addrBundle)
	set_target_properties(addrBundle PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR})
	target_compile_definitions(addrBundle PRIVATE WINDOWS_QADDR)

	if(USE_QML)
		qt6_add_qml_module(addrBundle
			URI  Esterv.Iota.AddrBundle
			VERSION 1.0
			QML_FILES
			"qml/TextAmount.qml"
			RESOURCE_PREFIX
			"/esterVtech.com/imports"
			OUTPUT_TARGETS out_targets_var
			OUTPUT_DIRECTORY
			${CMAKE_BINARY_DIR}/Esterv/Iota/AddrBundle
			IMPORT_PATH ${CMAKE_BINARY_DIR}
			)


		add_library(${PROJECT_NAME}::addrBundleplugin ALIAS addrBundleplugin)

		target_link_libraries(addrBundle PUBLIC Qt6::Gui Qt6::Quick)
		target_compile_definitions(addrBundle PRIVATE USE_QML)
		target_link_libraries(addrBundle PUBLIC
			$<$<STREQUAL:$<TARGET_PROPERTY:IotaAccount::account,TYPE>,STATIC_LIBRARY>:IotaAccount::accountplugin>
			$<$<STREQUAL:$<TARGET_PROPERTY:IotaNodeConn::nodeConnection,TYPE>,STATIC_LIBRARY>:IotaNodeConn::nodeConnectionplugin>
			)
		install(TARGETS ${out_targets_var}
			EXPORT ${PROJECT_NAME}-config
			DESTINATION ${CMAKE_INSTALL_LIBDIR}
			COMPONENT addrBundle
			)
		install(DIRECTORY ${CMAKE_BINARY_DIR}/Esterv/Iota/AddrBundle
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Iota
			COMPONENT addrBundle
			)
		install(TARGETS addrBundleplugin
			EXPORT ${PROJECT_NAME}-config
			DESTINATION ${CMAKE_INSTALL_LIBDIR}/Esterv/Iota/AddrBundle
			COMPONENT addrBundle
			)
	endif(USE_QML)
	target_link_libraries(addrBundle PUBLIC Qt6::Core)


	target_include_directories(addrBundle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Esterv/>")


	target_link_libraries(addrBundle PUBLIC IotaNodeConn::nodeConnection IotaAccount::account)


	install(TARGETS addrBundle
		EXPORT ${PROJECT_NAME}-config
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT addrBundle
		)

	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
		DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR}/Esterv
		COMPONENT ${PROJECT_NAME}
		)

	build_exports()
	if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
		build_cpack()
	endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
endif(Qt6_FOUND)

if(BUILD_DOCS)
	build_docs()
endif()
