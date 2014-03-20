USE_CYCLUS("BatchReactor" "batch_reactor")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibBatchReactor}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${BatchReactorTestSource} 
  PARENT_SCOPE
  )