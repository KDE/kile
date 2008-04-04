
########### install files ###############
FILE(GLOB kile_enc *.enc)
INSTALL( FILES ${kile_enc} DESTINATION ${DATA_INSTALL_DIR}/kile/encodings )
