
########### install files ###############
FILE(GLOB kile_jsplug *.js)
INSTALL( FILES ${kile_jsplug} DESTINATION ${DATA_INSTALL_DIR}/kile/scripts )
