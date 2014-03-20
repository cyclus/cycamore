USE_CYCLUS("NullRegion" "null_region")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibNullRegion}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${NullRegionTestSource} 
  PARENT_SCOPE
  )