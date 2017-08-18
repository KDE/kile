
########### install files ###############
file(GLOB kile_template *.tex)
install(FILES ${kile_template} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/templates)
