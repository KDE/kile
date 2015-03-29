file(GLOB symbols *.png)
set(symbolType "operators")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
