
########### install files ###############
FILE(GLOB kile_template *.tex) 
INSTALL( FILES ${kile_template} DESTINATION ${DATA_INSTALL_DIR}/kile/templates )
