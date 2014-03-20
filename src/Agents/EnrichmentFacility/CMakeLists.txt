USE_CYCLUS("EnrichmentFacility" "enrichment_facility")

SET(
  CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES}
  ${LibEnrichmentFacility}
  PARENT_SCOPE
  )

SET(
  TestSource ${TestSource}
  ${EnrichmentFacilityTestSource} 
  PARENT_SCOPE
  )