USE_CYCLUS("SinkFacility" "sink_facility")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibSinkFacility}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${SinkFacilityTestSource} 
  PARENT_SCOPE
  )