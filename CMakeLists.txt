cmake_minimum_required(VERSION 3.10)

# define the project
project(ilqvya_random VERSION 2.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

option(Random_BuildTests "Build the unit tests" ON)

# define project variables
set(ILQVYA_RANDOM_TARGET_NAME "random")
set(ILQVYA_RANDOM_PACKAGE_NAME ${PROJECT_NAME})
set(ILQVYA_RANDOM_TARGETS_FILENAME "${ILQVYA_RANDOM_PACKAGE_NAME}Targets.cmake")
set(ILQVYA_RANDOM_CONFIG_FILENAME "${ILQVYA_RANDOM_PACKAGE_NAME}Config.cmake")
set(ILQVYA_RANDOM_CONFIGVERSION_FILENAME "${ILQVYA_RANDOM_PACKAGE_NAME}ConfigVersion.cmake")
set(ILQVYA_RANDOM_CONFIG_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${ILQVYA_RANDOM_PACKAGE_NAME}")
set(ILQVYA_RANDOM_INCLUDE_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# create and configure the library target
add_library(${ILQVYA_RANDOM_TARGET_NAME} INTERFACE)
add_library(ilqvya::random ALIAS ${ILQVYA_RANDOM_TARGET_NAME})
add_library(ilqvya_random ALIAS ${ILQVYA_RANDOM_TARGET_NAME})
add_library(effolkronium::random ALIAS ${ILQVYA_RANDOM_TARGET_NAME})
add_library(effolkronium_random ALIAS ${ILQVYA_RANDOM_TARGET_NAME})

target_compile_features(${ILQVYA_RANDOM_TARGET_NAME} INTERFACE cxx_std_11)

target_include_directories(${ILQVYA_RANDOM_TARGET_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${ILQVYA_RANDOM_INCLUDE_DESTINATION}>)

if(Random_BuildTests AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
    enable_testing()
    add_subdirectory(test)
endif()

# generate a config and config version file for the package
include(CMakePackageConfigHelpers)
configure_package_config_file("cmake/config.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/${ILQVYA_RANDOM_CONFIG_FILENAME}"
                              INSTALL_DESTINATION ${ILQVYA_RANDOM_CONFIG_DESTINATION}
                              PATH_VARS ILQVYA_RANDOM_INCLUDE_DESTINATION)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${ILQVYA_RANDOM_CONFIGVERSION_FILENAME}"
                                 COMPATIBILITY SameMajorVersion)
    
# export the library target in build directory
export(TARGETS ${ILQVYA_RANDOM_TARGET_NAME}
       NAMESPACE ilqvya::
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${ILQVYA_RANDOM_TARGETS_FILENAME}")

# install library target and config files
install(TARGETS ${ILQVYA_RANDOM_TARGET_NAME}
        EXPORT ${ILQVYA_RANDOM_PACKAGE_NAME})
install(DIRECTORY "include/ilqvya"
        DESTINATION ${ILQVYA_RANDOM_INCLUDE_DESTINATION})
install(DIRECTORY "include/effolkronium"
        DESTINATION ${ILQVYA_RANDOM_INCLUDE_DESTINATION})
install(EXPORT ${ILQVYA_RANDOM_PACKAGE_NAME}
        NAMESPACE ilqvya::
        FILE ${ILQVYA_RANDOM_TARGETS_FILENAME}
        DESTINATION ${ILQVYA_RANDOM_CONFIG_DESTINATION})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ILQVYA_RANDOM_CONFIG_FILENAME}"
              "${CMAKE_CURRENT_BINARY_DIR}/${ILQVYA_RANDOM_CONFIGVERSION_FILENAME}"
        DESTINATION ${ILQVYA_RANDOM_CONFIG_DESTINATION})

