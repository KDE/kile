file(GLOB symbols *.png)
set(symbolType "misc-text")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
