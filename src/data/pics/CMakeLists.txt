
########### install files ###############
file(GLOB kileicons *.kileicon)

install(FILES kile_splash.png DESTINATION ${DATA_INSTALL_DIR}/kile/pics RENAME kile_splash.png)
install(FILES type_Default.png type_Empty.png type_Project.png DESTINATION ${DATA_INSTALL_DIR}/kile/pics)
install(FILES ${kileicons} DESTINATION ${DATA_INSTALL_DIR}/kile/pics)
