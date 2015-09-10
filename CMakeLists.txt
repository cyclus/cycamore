CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#taken from http://geant4.cern.ch/support/source/geant4/CMakeLists.txt
IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(STATUS "Cycamore requires an out-of-source build.")
    message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
    message(STATUS "CMakeCache.txt")
    message(STATUS "CMakeFiles")
    message(STATUS "Once these files are removed, create a separate directory")
    message(STATUS "and run CMake from there")
    message(FATAL_ERROR "in-source build detected")
ENDIF()

# This project name is cycamore.
PROJECT(CYCAMORE)

# quiets fortify_source warnings when not compiling with optimizations
# in linux distros where compilers were compiled with fortify_source enabled by
# default (e.g. Arch linux).
MESSAGE("${CMAKE_BUILD_TYPE}")
STRING(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
IF(NOT ${BUILD_TYPE} STREQUAL "release")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
ENDIF()
# no overflow warnings because of silly coin-ness
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overflow")

# Direct any out-of-source builds to this directory
SET(CYCAMORE_SOURCE_DIR ${CMAKE_SOURCE_DIR})

IF(NOT CYCLUS_DOC_ONLY)
    # Direct any binary installation paths to this directory
    SET(CYCAMORE_BINARY_DIR ${CMAKE_BINARY_DIR})

    # This makes all the libraries build as SHARED
    SET(BUILD_SHARED_LIBS true)

    # Setup build locations.
    IF(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CYCAMORE_BINARY_DIR}/bin)
    endif()
    IF(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CYCAMORE_BINARY_DIR}/lib)
    endif()
    IF(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CYCAMORE_BINARY_DIR}/lib)
    ENDIF()

    SET(CYCAMORE_EXECUTABLE_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    ENDIF("${isSystemDir}" STREQUAL "-1")

    # Tell CMake where the modules are
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_DIR}/share/cmake-2.8/Modules")
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CYCAMORE_SOURCE_DIR}/cmake)
    MESSAGE("CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

    # Find cyclus
    FIND_PACKAGE(Cyclus REQUIRED)
    SET(
      CYCAMORE_INCLUDE_DIRS 
      ${CYCAMORE_INCLUDE_DIRS} 
      ${CYCLUS_CORE_INCLUDE_DIR} # INSTALL_PATH/include/cyclus
      )
    SET(LIBS ${LIBS} ${CYCLUS_CORE_LIBRARIES})

    # Include macros installed with cyclus core
    INCLUDE(UseCyclus)

    # Find LibXML++ and dependencies
    FIND_PACKAGE(LibXML++ REQUIRED)
    SET(CYCAMORE_INCLUDE_DIRS ${CYCAMORE_INCLUDE_DIRS} ${LibXML++_INCLUDE_DIR} ${Glibmm_INCLUDE_DIRS} ${LibXML++Config_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${LibXML++_LIBRARIES})
    MESSAGE("--LIBS: ${LIBS}")

    MESSAGE("--LD_LIBRARY_PATH: $ENV{LD_LIBRARY_PATH}")

    # Include the boost header files, system, and filesystem libraries
    SET(Boost_USE_STATIC_LIBS       OFF)
    SET(Boost_USE_STATIC_RUNTIME    OFF)
    FIND_PACKAGE(Boost COMPONENTS filesystem system REQUIRED)
    SET(CYCAMORE_INCLUDE_DIRS ${CYCAMORE_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${Boost_FILESYSTEM_LIBRARY})
    SET(LIBS ${LIBS} ${Boost_SYSTEM_LIBRARY})

    # find lapack and link to it
    # note there is no include directory variable:
    # http://www.cmake.org/cmake/help/v3.0/module/FindLAPACK.html
    FIND_PACKAGE(LAPACK REQUIRED)
    set(LIBS ${LIBS} ${LAPACK_LIBRARIES})
    MESSAGE("\tFound LAPACK Libraries: ${LAPACK_LIBRARIES}")

    # Find HDF5
    FIND_PACKAGE(HDF5 REQUIRED)
    ADD_DEFINITIONS(${HDF5_DEFINITIONS})
    SET(CYCAMORE_INCLUDE_DIRS ${CYCAMORE_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
    set(LIBS ${LIBS} ${HDF5_LIBRARIES})
    MESSAGE("--    HDF5 Root: ${HDF5_ROOT}")
    MESSAGE("--    HDF5 Include directory: ${HDF5_INCLUDE_DIR}")
    MESSAGE("--    HDF5 Library directories: ${HDF5_LIBRARY_DIRS}")
    MESSAGE("--    HDF5 Libraries: ${HDF5_LIBRARIES}")

    # find coin and link to it
    FIND_PACKAGE(COIN REQUIRED)
    SET(CYCAMORE_INCLUDE_DIRS ${CYCAMORE_INCLUDE_DIRS} ${COIN_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${COIN_LIBRARIES})
    MESSAGE("--    COIN Root: ${COIN_ROOT}")
    MESSAGE("--    COIN Include directories: ${COIN_INCLUDE_DIRS}")
    MESSAGE("--    COIN Libraries: ${COIN_LIBRARIES}")

    FIND_PACKAGE( Sqlite3 REQUIRED )
    SET(CYCAMORE_INCLUDE_DIRS ${CYCAMORE_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIR})
    SET(LIBS ${LIBS} ${SQLITE3_LIBRARIES})
    MESSAGE("--    SQLITE3 Include directories: ${SQLITE3_INCLUDE_DIR}")
    MESSAGE("--    SQLITE3 Libraries: ${SQLITE3_LIBRARIES}")

    # include all the directories we just found
    INCLUDE_DIRECTORIES(${CYCAMORE_INCLUDE_DIRS})

    # ------------------------- Add the Agents -----------------------------------
    ADD_SUBDIRECTORY(src)

    # ------------------------- Google Test -----------------------------------

    # Be sure to clear these each time
    SET(GENERATED_TEST_LIST "" CACHE INTERNAL "")
    SET(GENERATED_FILTER_LIST "" CACHE INTERNAL "")
    SET(GENERATED_TEST_SOURCE_LIST "" CACHE INTERNAL "")

    # Create an executable for all the gtests
    # Options for testing
    OPTION(USE_TESTING "Build testing" ON)
    IF(USE_TESTING)
        enable_testing()
        INCLUDE(CTest)
    ENDIF()

    ADD_SUBDIRECTORY(tests)

    SET(TestSource
        ${TestSource}
        ${CYCAMORE_TEST_CORE}
        )

    FIND_PACKAGE(Threads)
    IF(CMAKE_USE_PTHREADS_INIT)  # The pthreads library is available.
        SET(cxx_base_flags "${cxx_base_flags} -DGTEST_HAS_PTHREAD=1")
    ENDIF()

    # Build cycamore_unit_tests
    ADD_EXECUTABLE(cycamore_unit_tests
        tests/cycamore_unit_test_driver.cc
        ${TestSource}
        )

    TARGET_LINK_LIBRARIES(cycamore_unit_tests
        dl ${LIBS} ${CYCLUS_TEST_LIBRARIES})

    INSTALL(TARGETS cycamore_unit_tests
        RUNTIME DESTINATION bin
        COMPONENT testing
        )

    # read tests after building the driver, and add them to ctest
    set(tgt "cycamore_unit_tests")
    set(script "${CYCAMORE_SOURCE_DIR}/config/generate_test_macros.py")
    set(exec "--executable=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${tgt}")
    set(reg_dir "--reg_dir=${CYCAMORE_SOURCE_DIR}/../${test_dir}")
    set(out "--output=${CMAKE_BINARY_DIR}/CTestTestfile.cmake")
    add_custom_command(TARGET ${tgt}
        POST_BUILD
        COMMAND python ${script} ${exec} ${reg_dir} ${out}
        COMMENT "adding tests from ${tgt}"
        DEPENDS
        VERBATIM
        )

    # for post test commands
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config/CTestCustom.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
        IMMEDIATE @ONLY)

    # for printing regression tests
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config/print_regs.py"
        "${CMAKE_CURRENT_BINARY_DIR}/print_regs.py"
        IMMEDIATE @ONLY)

    # AMS: I don't believe that this is needed anymore.
    # This just copies the shared dir from cyclus to cycamore.
    # However, the 'share/' dir should now be the same.  So this
    # was failing because you can't copy a file to itself.
    #FILE(GLOB cyclus_shared "${CYCLUS_CORE_SHARE_DIR}/*")
    #INSTALL(FILES ${cyclus_shared}
    #  DESTINATION share
    #  COMPONENT data
    # )

    # Here we set some components for installation with cpack
    SET(CPACK_COMPONENTS_ALL cycamore testing libraries data core)

    # Here's where we package it with CPack
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cycamore : The Cyclus Additional Module Repository.")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "CNERG, UW-Madison") #required
    SET(CPACK_PACKAGE_VENDOR "CNERG, UW-Madison")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.rst")
    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rst")
    SET(CPACK_PACKAGE_VERSION_MAJOR "1")
    SET(CPACK_PACKAGE_VERSION_MINOR "0")
    #SET(CPACK_PACKAGE_VERSION_PATCH "0") # should use commit number here  ?
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "cycamore${CPACK_VERSION_MAJOR}.${CPACK_VERSION_MINOR}")
    SET(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")

    CONFIGURE_FILE("${CYCAMORE_SOURCE_DIR}/cmake/CycamoreCPackOptions.cmake.in"
        "${CYCAMORE_BINARY_DIR}/cmake/CycamoreCPackOptions.cmake" @ONLY)
    SET(CPACK_PROJECT_CONFIG_FILE "${CYCAMORE_BINARY_DIR}/cmake/CycamoreCPackOptions.cmake")
    SET(CPACK_PACKAGE_EXECUTABLES "cycamore" "cycamore_unit_tests")


    INCLUDE(CPack)

    SET(ROOT_DIR ${CYCLUS_ROOT_DIR})
    SET(PROJ_DIR ${PROJECT_BINARY_DIR})
    SET(CORE_SHARE_DIR ${CYCLUS_CORE_SHARE_DIR})

ENDIF(NOT CYCLUS_DOC_ONLY)

# This is the directory that holds the doxygen doxyfile template (doxy.conf.in)
SET(DOC_INPUT_DIR ${CYCAMORE_SOURCE_DIR}/doc)

# If doxygen exists, use the doc/CMakeLists.txt file for further instructions.
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    ADD_SUBDIRECTORY(doc)
    SET(DOC_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc)
ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS "WARNING: Doxygen not found - doc won't be created")
ENDIF(DOXYGEN_FOUND)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
