
########### install files ###############
file(GLOB kileicons *.kileicon)

install(FILES kile_splash.png DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/pics RENAME kile_splash.png)
install(FILES type_Default.png type_Empty.png type_Project.png DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/pics)
install(FILES ${kileicons} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/pics)
