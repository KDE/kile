file(GLOB symbols *.png)
set(symbolType "misc-math")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
