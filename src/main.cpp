#include <QApplication>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QStringList argumentList = app.arguments();
	argumentList.removeFirst();
	if (!argumentList.isEmpty())
		return 0;
	MainWindow mainWin;
	app.setApplicationName(mainWin.getAppTitle());
	mainWin.switchLanguage();
	mainWin.show();
	if (!mainWin.isBackEndAvailable()){
		mainWin.showNoBackEndVersion(true);
	}
	return app.exec();
}

