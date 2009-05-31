file(GLOB symbols *.png)
SET(symbolType "operators")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
