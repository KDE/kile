
########### install files ###############
file(GLOB kile_enc *.enc)
install(FILES ${kile_enc} DESTINATION ${DATA_INSTALL_DIR}/kile/encodings)
