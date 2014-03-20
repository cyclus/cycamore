USE_CYCLUS("NullInst" "null_inst")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibNullInst}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${NullInstTestSource} 
  PARENT_SCOPE
  )