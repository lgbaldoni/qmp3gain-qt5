#include <QtGui>

#include "advancedoptionsdialog.h"
#include "mainwindow.h"

AdvancedOptionsDialog::AdvancedOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	
	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	//QRegExp regExp("[A-Za-z][1-9][0-9]{0,2}");
	//lineEdit->setValidator(new QRegExpValidator(regExp, this));
	
	//connect(donationButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	// fill fields vector with checkable control elements
	{
		fields << checkBox_Maximizing;
		fields << horizontalSlider_logBackendDepth;
		fields << horizontalSlider_logTraceDepth;
		fields << checkBox_ShowHiddenFields;
		fields << checkBox_UseNoTempFiles;
		fields << checkBox_ShowNoFileProgress;
	}
	
	MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parent());
	readSettings(mainWindow->getSettings());
	
	isAccepted = false;
	horizontalSlider_logBackendDepth_origValue = horizontalSlider_logBackendDepth->value();
	horizontalSlider_logTraceDepth_origValue = horizontalSlider_logTraceDepth->value();
	}

AdvancedOptionsDialog::~AdvancedOptionsDialog(){
	if (isAccepted){
		MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parent());
		writeSettings(mainWindow->getSettings());
		if (horizontalSlider_logBackendDepth->value() != horizontalSlider_logBackendDepth_origValue){
			mainWindow->actionLogBackend->setChecked(horizontalSlider_logBackendDepth->value() > 0);
		}
		if (horizontalSlider_logTraceDepth->value() != horizontalSlider_logTraceDepth_origValue){
			mainWindow->actionLogTrace->setChecked(horizontalSlider_logTraceDepth->value() > 0);
		}
	}
}

void AdvancedOptionsDialog::readSettings(QSettings *settings){
	settings->beginGroup("advancedOptionsDialog");
	for (int i=0; i<fields.count(); i++){
		if (fields[i]->objectName().startsWith("horizontalSlider"))
			(qobject_cast<QSlider*>(fields[i]))->setValue(settings->value(fields[i]->objectName(), (qobject_cast<QSlider*>(fields[i]))->value()).toInt());
		else if (fields[i]->objectName().startsWith("checkBox"))
			(qobject_cast<QCheckBox*>(fields[i]))->setChecked(settings->value(fields[i]->objectName(), (qobject_cast<QCheckBox*>(fields[i]))->isChecked()).toBool());
	}
	settings->endGroup();
}

void AdvancedOptionsDialog::writeSettings(QSettings *settings){
	settings->beginGroup("advancedOptionsDialog");
	for (int i=0; i<fields.count(); i++){
		if (fields[i]->objectName().startsWith("horizontalSlider"))
			settings->setValue(fields[i]->objectName(), (qobject_cast<QSlider*>(fields[i]))->value());
		else if (fields[i]->objectName().startsWith("checkBox"))
			settings->setValue(fields[i]->objectName(), (qobject_cast<QCheckBox*>(fields[i]))->isChecked());
	}
	settings->endGroup();
}

void AdvancedOptionsDialog::accept(){
	isAccepted = true;
	return QDialog::accept();
}
