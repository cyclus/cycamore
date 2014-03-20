USE_CYCLUS("SourceFacility" "source_facility")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibSourceFacility}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${SourceFacilityTestSource} 
  PARENT_SCOPE
  )