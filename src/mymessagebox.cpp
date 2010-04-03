/*
 * MyMessageBox.cpp
 *
 *  Created on: 2009.01.30.
 *      Author: brazso
 */

#include <QtGui>
#include "mymessagebox.h"

MyMessageBox::MyMessageBox() {
	// TODO Auto-generated constructor stub
}

MyMessageBox::~MyMessageBox() {
	// TODO Auto-generated destructor stub
}

QMessageBox::StandardButton MyMessageBox::messageBox ( QMessageBox::Icon icon, QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton){
	QMessageBox msgBox(parent);
	msgBox.setWindowTitle(title);
	msgBox.setText(text);
	//msgBox.setInformativeText("Do you want to save your changes?");
	msgBox.setStandardButtons(buttons);
	msgBox.setDefaultButton(defaultButton);
	msgBox.setIcon(icon);
	QCheckBox* checkBox = 0;

	if (!isConfirmSuppressed && !confirmText.isEmpty()){
		QGridLayout* grid = qobject_cast<QGridLayout*>(msgBox.layout());
		if (grid){
			checkBox = new QCheckBox(confirmText);
			grid->addWidget(checkBox, grid->rowCount()-2, 0, 1, grid->columnCount());
		}
	}

	if (msgBox.exec() == -1)
		return QMessageBox::Cancel;

	if (checkBox){
		isConfirmSuppressed = checkBox->isChecked();
	}

	return msgBox.standardButton(msgBox.clickedButton());
}

QMessageBox::StandardButton MyMessageBox::critical ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton){
	return messageBox(QMessageBox::Critical, parent, title, text, confirmText, isConfirmSuppressed, buttons, defaultButton);
}

QMessageBox::StandardButton MyMessageBox::information ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton){
	return messageBox(QMessageBox::Information, parent, title, text, confirmText, isConfirmSuppressed, buttons, defaultButton);
}

QMessageBox::StandardButton MyMessageBox::question ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton){
	return messageBox(QMessageBox::Question, parent, title, text, confirmText, isConfirmSuppressed, buttons, defaultButton);
}

QMessageBox::StandardButton MyMessageBox::warning ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton){
	return messageBox(QMessageBox::Warning, parent, title, text, confirmText, isConfirmSuppressed, buttons, defaultButton);
}
