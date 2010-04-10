#include <QtGui>

#include "disclaimerdialog.h"
#include "mainwindow.h"

DisclaimerDialog::DisclaimerDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	
	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	//connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}
