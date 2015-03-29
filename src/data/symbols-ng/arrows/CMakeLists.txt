file(GLOB symbols *.png)
set(symbolType "arrows")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
