
########### install files ###############
file(GLOB kileicons *.kileicon)

INSTALL(FILES kile_splash.png DESTINATION ${DATA_INSTALL_DIR}/kile/pics RENAME kile_splash.png)

INSTALL(FILES  type_Default.png type_Empty.png type_Project.png DESTINATION ${DATA_INSTALL_DIR}/kile/pics)
INSTALL(FILES ${kileicons} DESTINATION ${DATA_INSTALL_DIR}/kile/pics)
