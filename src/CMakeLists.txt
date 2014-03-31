# ------------------- Add all Concrete Agents ----------------------------

MACRO(ADD_AGENT LIB_ROOT SRC_ROOT)
  
  USE_CYCLUS(${LIB_ROOT} ${SRC_ROOT})

  SET(
    TestSource ${TestSource}
    ${${LIB_ROOT}_TEST_CC}
    ${${LIB_ROOT}_CC}
    )

ENDMACRO()

ADD_AGENT("BatchReactor" "batch_reactor")

ADD_AGENT("EnrichmentFacility" "enrichment_facility")

#ADD_AGENT("InproReactor" "inpro_reactor")

ADD_AGENT("SinkFacility" "sink_facility")

ADD_AGENT("SourceFacility" "source_facility")

ADD_AGENT("DeployInst" "deploy_inst")

#ADD_AGENT("ManagerInst" "manager_inst")

#ADD_AGENT("GrowthRegion" "growth_region")

SET( TestSource ${TestSource} PARENT_SCOPE)
