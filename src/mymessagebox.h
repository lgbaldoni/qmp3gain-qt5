/*
 * MyMessageBox.h
 *
 *  Created on: 2009.01.30.
 *      Author: brazso
 */

#ifndef MYMESSAGEBOX_H_
#define MYMESSAGEBOX_H_

#include <QMessageBox>

class MyMessageBox: public QMessageBox {
public:
	MyMessageBox();
	virtual ~MyMessageBox();

	static QMessageBox::StandardButton critical ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirm, StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
	static QMessageBox::StandardButton information ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirm, StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
	static QMessageBox::StandardButton question ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirm, StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);
	static QMessageBox::StandardButton warning ( QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirm, StandardButtons buttons = Ok, QMessageBox::StandardButton defaultButton = NoButton);

private:
	static QMessageBox::StandardButton messageBox ( QMessageBox::Icon icon, QWidget * parent, const QString & title, const QString & text, const QString & confirmText, bool & isConfirmSuppressed, StandardButtons buttons, QMessageBox::StandardButton defaultButton);
};

#endif /* MYMESSAGEBOX_H_ */
