
########### install files ###############
file(GLOB kile_jsplug *.js)
install(FILES ${kile_jsplug} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/script-plugins)
