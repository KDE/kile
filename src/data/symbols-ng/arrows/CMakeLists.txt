file(GLOB symbols *.png)
SET(symbolType "arrows")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
