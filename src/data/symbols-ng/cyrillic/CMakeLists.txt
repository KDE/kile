file(GLOB symbols *.png)
SET(symbolType "cyrillic")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
