file(GLOB symbols *.png)
set(symbolType "misc-math")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
