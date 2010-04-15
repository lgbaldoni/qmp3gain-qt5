#include <QtGui>

#include "aboutdialog.h"
#include "mainwindow.h"
#include "donationdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	
	MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parent());
	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	//QRegExp regExp("[A-Za-z][1-9][0-9]{0,2}");
	//lineEdit->setValidator(new QRegExpValidator(regExp, this));
	
	//connect(donationButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	// english makes translationLabel invisible
	translationLabel->setVisible(false);
	//translationVerticalSpacer->setVisible(false);
	
	// set appTitle, e.g. "%1 (GUI)" -> "QMP3Gain (GUI)"
	appTitleLabel->setText(appTitleLabel->text().arg(MainWindow::getAppTitle()));
	
	// add appVersion to label, e.g. "Version %1" -> "Version 1.3.4"
	appVersionLabel->setText(appVersionLabel->text().arg(MainWindow::getAppVersion()));
	
	// add backEnd to label
	QFileInfo fi(mainWindow->getBackEnd());
	backEndLabel->setText(backEndLabel->text().arg(fi.baseName()));

	// add backEndVersion to label
	backEndVersionLabel->setText(backEndVersionLabel->text().arg(mainWindow->getBackEndVersion()));
	
	latestAppVersionAtLabel->setText(latestAppVersionAtLabel->text().arg(MainWindow::getAppTitle()));
}

void AboutDialog::on_donationButton_clicked() {
	// modal dialog
	DonationDialog donationDialog(this);
	if (donationDialog.exec()) {
		this->accept();
	}
}
