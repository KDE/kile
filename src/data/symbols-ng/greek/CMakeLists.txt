file(GLOB symbols *.png)
set(symbolType "greek")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
