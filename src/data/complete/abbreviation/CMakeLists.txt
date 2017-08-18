file(GLOB kile_abbrevFiles *.cwl)
install(FILES ${kile_abbrevFiles} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/complete/abbreviation)
