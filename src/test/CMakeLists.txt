
########### install files ###############

SET( testFiles
	test.tex
	test_plain.tex
	test.bib
	test_bib.tex
	test_index.tex
)

IF(CMAKE_SYSTEM_NAME MATCHES Windows)
  SET( testFiles
	 ${testFiles}
	 runTests.bat
	)

ELSE(CMAKE_SYSTEM_NAME MATCHES Windows)
  SET( testFiles
	 ${testFiles}
	 runTests.sh
	)

ENDIF(CMAKE_SYSTEM_NAME MATCHES Windows)

# INSTALL( FILES ${testFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/test )
