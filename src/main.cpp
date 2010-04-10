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
	mainWin.switchLanguage();
	mainWin.show();
	if (mainWin.getBackEndVersion().isEmpty()){
		mainWin.showNoBackEndVersion(true);
	}
	return app.exec();
}

