USE_CYCLUS("DeployInst" "deploy_inst")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibDeployInst}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${DeployInstTestSource} 
  PARENT_SCOPE
  )