# included by ../src/src.pro

!isEmpty(HELPS) {
	# qhelpgenerator for generation from QHP to QCH
	# qhelpgenerator qmp3gain.qhp -o qmp3gain.qch

	isEmpty(QMAKE_QHELPGENERATOR) {
		win32:QMAKE_QHELPGENERATOR = $$[QT_INSTALL_BINS]\qhelpgenerator.exe
		else:QMAKE_QHELPGENERATOR = $$[QT_INSTALL_BINS]/qhelpgenerator
	}

	isEmpty(HELP_DIR):HELP_DIR = ../help

	for(file, HELPS) {
		exists($${file}.qhp) {
		HELPS_QHP += $${file}.qhp
		}
	}

	QHPQCH.name = qhelpgenerator ${QMAKE_FILE_IN}
	QHPQCH.input = HELPS_QHP
	QHPQCH.output = $$HELP_DIR/${QMAKE_FILE_BASE}.qch
	QHPQCH.commands = $$QMAKE_QHELPGENERATOR ${QMAKE_FILE_IN}
	QHPQCH.CONFIG = no_link
	QMAKE_EXTRA_COMPILERS += QHPQCH
	PRE_TARGETDEPS += compiler_QHPQCH_make_all

	# qcollectiongenerator for generation from QHCP to QHC
	# qcollectiongenerator qmp3gain.qhcp -o qmp3gain.qhc

	isEmpty(QMAKE_QCOLLECTIONGENERATOR) {
		win32:QMAKE_QCOLLECTIONGENERATOR = $$[QT_INSTALL_BINS]\qcollectiongenerator.exe
		else:QMAKE_QCOLLECTIONGENERATOR = $$[QT_INSTALL_BINS]/qcollectiongenerator
	}

	for(file, HELPS) {
		exists($${file}.qhcp) {
		HELPS_QHCP += $${file}.qhcp
		}
	}

	QHCPQHC.name = qcollectiongenerator ${QMAKE_FILE_IN}
	QHCPQHC.input = HELPS_QHCP
	QHCPQHC.output = $$HELP_DIR/${QMAKE_FILE_BASE}.qhc
	QHCPQHC.commands = $$QMAKE_QCOLLECTIONGENERATOR ${QMAKE_FILE_IN}
	QHCPQHC.depends = compiler_QHPQCH_make_all
	QHCPQHC.CONFIG = no_link
	QMAKE_EXTRA_COMPILERS += QHCPQHC
	PRE_TARGETDEPS += compiler_QHCPQHC_make_all

	win32 {
		help.path = $${INSTALLDIR}/help
		help.files = $$HELP_DIR/*.qhc $$HELP_DIR/*.qch
		INSTALLS += help
	}
	else {
		help.path = $${INSTALLDIR_SHARE}/help
		help.files = $$HELP_DIR/*.qhc $$HELP_DIR/*.qch
		INSTALLS += help
	}

} else:message(No help files in project)
