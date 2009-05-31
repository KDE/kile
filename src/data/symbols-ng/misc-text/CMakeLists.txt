file(GLOB symbols *.png)
SET(symbolType "misc-text")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
