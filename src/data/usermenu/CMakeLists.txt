
########### install files ###############
FILE(GLOB kile_lum *.xml)
INSTALL( FILES ${kile_lum} DESTINATION ${DATA_INSTALL_DIR}/kile/usermenu )
