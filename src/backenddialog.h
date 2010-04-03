#ifndef BACKENDDIALOG_H
#define BACKENDDIALOG_H

#include <QDialog>

#include "ui_backenddialog.h"
#include "mainwindow.h"

class BackEndDialog : public QDialog, public Ui::BackEndDialog
{
    Q_OBJECT

public:
	BackEndDialog(QWidget *parent = 0);
	~BackEndDialog();

private:
	bool isAccepted;
	void refreshStatusAndVersion(bool isInit);
	MainWindow* mainWindow;
	QString label_Status_Orig;
	QString label_Version_Orig;

private slots:
	void accept();
	void on_toolButton_fileName_clicked();
	void on_lineEdit_fileName_editingFinished();
};

#endif

