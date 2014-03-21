SET(LIB_ROOT "EnrichmentFacility")
SET(SRC_ROOT "enrichment_facility") 

USE_CYCLUS(${LIB_ROOT} ${SRC_ROOT})

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LIB_ROOT}_LIB
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${${LIB_ROOT}_TEST_CC}
  PARENT_SCOPE
  )