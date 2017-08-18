file(GLOB symbols *.png)
set(symbolType "operators")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
