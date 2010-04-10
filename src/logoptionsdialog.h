#ifndef LOGOPTIONSDIALOG_H
#define LOGOPTIONSDIALOG_H

#include <QDialog>
#include <QSettings>

#include "ui_logoptionsdialog.h"
#include "mainwindow.h"

class LogOptionsDialog : public QDialog, public Ui::LogOptionsDialog
{
    Q_OBJECT

public:
	LogOptionsDialog(QWidget *parent = 0);
	~LogOptionsDialog();

private:
	bool isAccepted;
	void readSettings(QSettings *settings);
	void writeSettings(QSettings *settings);
	void openFileDialog(QLineEdit *lineEdit);
	MainWindow* mainWindow;

private slots:
	void accept();
	void on_toolButton_logFile_clicked();
	void on_buttonBox_clicked(QAbstractButton * button);
};

#endif

