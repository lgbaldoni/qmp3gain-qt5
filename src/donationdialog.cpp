#include <QtGui>
#include <QWebView>

#include "donationdialog.h"
#include "mainwindow.h"

DonationDialog::DonationDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);

	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	//connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

void DonationDialog::on_donationButton_clicked() {
	MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parent()->parent());
	if (mainWindow->getDonationView())
		delete mainWindow->getDonationView();
	QPointer<QWebView> view = new QWebView();
	mainWindow->setDonationView(view);
	//mainWindow->statusbar->showMessage(tr("Opening donation url..."));
	connect( view, SIGNAL(loadStarted()), mainWindow, SLOT(loadDonationUrlStarted()) );
	connect( view, SIGNAL(loadProgress (int)), mainWindow, SLOT(loadDonationUrlProgress(int)) );
	connect( view, SIGNAL(loadFinished(bool)), mainWindow, SLOT(loadDonationUrlFinished(bool)) );
	//view->load(QUrl("http://qt.nokia.com/")); // works
	//view->load(QUrl("http://zematix.hu/")); // works
	view->load(QUrl(MainWindow::getDonationUrl()));
	//view->show(); // it is called in mainWindow's activateDonationWindow() function
	this->accept();
}
