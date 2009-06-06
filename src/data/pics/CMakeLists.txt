
########### install files ###############
file(GLOB kileicons *.kileicon *.png)
INSTALL(FILES ${kileicons} DESTINATION ${DATA_INSTALL_DIR}/kile/pics)
