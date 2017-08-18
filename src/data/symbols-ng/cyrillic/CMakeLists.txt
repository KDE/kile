file(GLOB symbols *.png)
set(symbolType "cyrillic")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
