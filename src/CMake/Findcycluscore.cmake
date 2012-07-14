#  CYCLUS_CORE_FOUND - system has the Cyclus Core library
#  CYCLUS_CORE_INCLUDE_DIR - the Cyclus include directory
#  CYCLUS_CORE_LIBRARIES - The libraries needed to use the Cyclus Core Library

IF(NOT DEFINED CYCLUS_ROOT_DIR)
  SET(CYCLUS_ROOT_DIR "$ENV{CYCLUS_ROOT_DIR}")
ENDIF(NOT DEFINED CYCLUS_ROOT_DIR)

# Look for the header files
FIND_PATH(CYCLUS_CORE_INCLUDE_DIR suffix.h
  HINTS "${CYCLUS_ROOT_DIR}" "${CYCLUS_ROOT_DIR}/cyclus" 
  /usr/local/cyclus /opt/local/cyclus 
  PATH_SUFFIXES cyclus/include include)

MESSAGE(STATUS "CYCLUS_ROOT_DIR hint is : ${CYCLUS_ROOT_DIR}")

# Add the root dir to the hints
IF(CYCLUS_ROOT_DIR MATCHES "/")
ELSE(CYCLUS_ROOT_DIR MATCHES "/")
  SET(CYCLUS_ROOT_DIR "${CYCLUS_CORE_INCLUDE_DIR}/..")
  MESSAGE(STATUS "CYCLUS_ROOT_DIR hint has been set to : ${CYCLUS_ROOT_DIR}")
ENDIF(CYCLUS_ROOT_DIR MATCHES "/")


# Look for the header files
FIND_PATH(CYCLUS_CORE_SHARE_DIR cyclus.rng.in
  HINTS "${CYCLUS_ROOT_DIR}" "${CYCLUS_ROOT_DIR}/cyclus" 
  /usr/local/cyclus /opt/local/cyclus
  PATH_SUFFIXES cyclus/share share)

# Look for the library
FIND_LIBRARY(CYCLUS_CORE_LIBRARY NAMES cycluscore 
  HINTS "${CYCLUS_ROOT_DIR}" "${CYCLUS_ROOT_DIR}/cyclus" 
  /usr/local/cyclus/lib /usr/local/cyclus 
  /opt/local /opt/local/cyclus
  PATH_SUFFIXES cyclus/lib lib)

# Look for the library
FIND_LIBRARY(CYCLUS_GTEST_LIBRARY NAMES gtest
  HINTS "${CYCLUS_ROOT_DIR}" "${CYCLUS_ROOT_DIR}/cyclus" 
  /usr/local/cyclus/lib /usr/local/cyclus 
  /opt/local/lib /opt/local/cyclus/lib 
  PATH_SUFFIXES cyclus/lib lib)

# Copy the results to the output variables.
IF (CYCLUS_CORE_INCLUDE_DIR AND CYCLUS_CORE_LIBRARY AND CYCLUS_GTEST_LIBRARY AND CYCLUS_CORE_SHARE_DIR)
	SET(CYCLUS_CORE_FOUND 1)
        SET(CYCLUS_CORE_LIBRARIES ${CYCLUS_CORE_LIBRARY})
        SET(CYCLUS_GTEST_LIBRARIES ${CYCLUS_GTEST_LIBRARY})
        SET(CYCLUS_CORE_INCLUDE_DIRS ${CYCLUS_CORE_INCLUDE_DIR})
        SET(CYCLUS_CORE_SHARE_DIRS ${CYCLUS_CORE_SHARE_DIR})
ELSE (CYCLUS_CORE_INCLUDE_DIR AND CYCLUS_CORE_LIBRARY AND CYCLUS_GTEST_LIBRARY AND CYCLUS_CORE_SHARE_DIR)
	SET(CYCLUS_CORE_FOUND 0)
	SET(CYCLUS_CORE_LIBRARIES)
        SET(CYCLUS_GTEST_LIBRARIES)
	SET(CYCLUS_CORE_INCLUDE_DIRS)
        SET(CYCLUS_CORE_SHARE_DIRS)
ENDIF (CYCLUS_CORE_INCLUDE_DIR AND CYCLUS_CORE_LIBRARY AND CYCLUS_GTEST_LIBRARY AND CYCLUS_CORE_SHARE_DIR)

# Report the results.
IF (CYCLUS_CORE_FOUND)
  SET(CYCLUS_CORE_DIR_MESSAGE "Found Cyclus Core Headers : " 
    ${CYCLUS_CORE_INCLUDE_DIRS} )
  SET(CYCLUS_CORE_SHARE_MESSAGE "Found Cyclus Core Shared Data : " 
    ${CYCLUS_CORE_SHARE_DIRS} )
  SET(CYCLUS_CORE_LIB_MESSAGE "Found Cyclus Core Library : " 
    ${CYCLUS_CORE_LIBRARIES} )
  SET(CYCLUS_GTEST_LIB_MESSAGE "Found Cyclus GTest Library : " 
    ${CYCLUS_GTEST_LIBRARIES} )
  MESSAGE(STATUS ${CYCLUS_CORE_DIR_MESSAGE}) 
  MESSAGE(STATUS ${CYCLUS_CORE_SHARE_MESSAGE}) 
  MESSAGE(STATUS ${CYCLUS_CORE_LIB_MESSAGE}) 
  MESSAGE(STATUS ${CYCLUS_GTEST_LIB_MESSAGE}) 
ELSE (CYCLUS_CORE_FOUND)
	SET(CYCLUS_CORE_DIR_MESSAGE
		"Cyclus was not found. Make sure CYCLUS_CORE_LIBRARY and CYCLUS_CORE_INCLUDE_DIR are set.")
	IF (NOT CYCLUS_CORE_FIND_QUIETLY)
		MESSAGE(STATUS "${CYCLUS_CORE_DIR_MESSAGE}")
	ELSE (NOT CYCLUS_CORE_FIND_QUIETLY)
		IF (CYCLUS_CORE_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "${CYCLUS_CORE_DIR_MESSAGE}")
		ENDIF (CYCLUS_CORE_FIND_REQUIRED)
	ENDIF (NOT CYCLUS_CORE_FIND_QUIETLY)
ENDIF (CYCLUS_CORE_FOUND)

MARK_AS_ADVANCED(
	CYCLUS_CORE_INCLUDE_DIR
	CYCLUS_CORE_LIBRARY
)
