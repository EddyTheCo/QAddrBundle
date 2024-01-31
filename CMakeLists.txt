cmake_minimum_required(VERSION 3.24)

option(USE_QML "Create QML types and modules" OFF)
include(${CMAKE_CURRENT_BINARY_DIR}/local_conf.cmake OPTIONAL)
include(FetchContent)
FetchContent_Declare(
    ccommon
    GIT_REPOSITORY https://github.com/EddyTheCo/Common.git
    GIT_TAG v0.1.1
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

FetchContent_Declare(
    IotaNodeConn
    GIT_REPOSITORY https://github.com/EddyTheCo/ConectionSettings.git
    GIT_TAG develop
    FIND_PACKAGE_ARGS 0.5 CONFIG
)
FetchContent_MakeAvailable(IotaNodeConn)

FetchContent_Declare(
    IotaAccount
    GIT_REPOSITORY https://github.com/EddyTheCo/account.git
    GIT_TAG develop
    FIND_PACKAGE_ARGS 0.5 CONFIG
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
if (Qt6_FOUND)
    target_link_libraries(addrBundle PUBLIC Qt6::Core)
endif(Qt6_FOUND)


target_include_directories(addrBundle PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Esterv/${PROJECT_NAME}>")


target_link_libraries(addrBundle PUBLIC IotaNodeConn::nodeConnection IotaAccount::account)


install(TARGETS addrBundle
    EXPORT ${PROJECT_NAME}-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT addrBundle
)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR}/Esterv/${PROJECT_NAME}
    COMPONENT addrBundle
)

install(EXPORT ${PROJECT_NAME}-config
    FILE ${PROJECT_NAME}-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
    NAMESPACE ${PROJECT_NAME}::
    COMPONENT ${PROJECT_NAME}
)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esterv
)
export(EXPORT ${PROJECT_NAME}-config
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
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
endif(Qt6_FOUND)

if(BUILD_DOCS)
    get_target_property(build_docs cmake_build_docs SOURCES)
    include(${build_docs})
endif()
