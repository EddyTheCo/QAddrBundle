cmake_minimum_required(VERSION 3.16)
project(IOTA-addresses VERSION 0.2 LANGUAGES CXX)

include(local_conf.cmake OPTIONAL)

find_package(Qt6 REQUIRED COMPONENTS Core )

add_library(qaddr_bundle qaddr_bundle.cpp include/qaddr_bundle.hpp)

target_include_directories(qaddr_bundle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	"$<INSTALL_INTERFACE:include>")
target_link_libraries(qaddr_bundle PUBLIC Qt6::Core)
include(FetchContent)
FetchContent_Declare(
        qclient
        GIT_REPOSITORY git@github.com:EddyTheCo/Qclient-IOTA.git
        GIT_TAG version-0.2
        )
if(NOT TARGET qclient)
FetchContent_MakeAvailable(qclient)
endif(NOT TARGET qclient)
FetchContent_Declare(
	qbech32
	GIT_REPOSITORY git@github.com:EddyTheCo/Qbech32.git
	GIT_TAG main
	)
FetchContent_MakeAvailable(qbech32)
FetchContent_Declare(
        qed25519
        GIT_REPOSITORY git@github.com:EddyTheCo/Qed25519.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(qed25519)

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
	VERSION "0.1.1.0"
	COMPATIBILITY AnyNewerVersion
	)
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleConfigVersion.cmake
	DESTINATION lib/cmake/qaddr_bundle
	)
export(EXPORT qaddr_bundleTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/qaddr_bundleTargets.cmake"
	)
