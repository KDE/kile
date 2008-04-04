
########### install files ###############
FILE(GLOB kile_lst *.lst)
INSTALL( FILES ${kile_lst} DESTINATION ${DATA_INSTALL_DIR}//kile/help )
