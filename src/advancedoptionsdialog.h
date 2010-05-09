#ifndef ADVANCEDOPTIONSDIALOG_H
#define ADVANCEDOPTIONSDIALOG_H

#include <QDialog>
#include <QSettings>

#include "ui_advancedoptionsdialog.h"

class AdvancedOptionsDialog : public QDialog, public Ui::AdvancedOptionsDialog
{
    Q_OBJECT

public:
	AdvancedOptionsDialog(QWidget *parent = 0);
	~AdvancedOptionsDialog();

private:
	bool isAccepted;
	int horizontalSlider_logBackendDepth_origValue;
	int horizontalSlider_logTraceDepth_origValue;
	int spinBox_beepAfter_origValue;
	QVector<QObject*> fields;
	void readSettings(QSettings *settings);
	void writeSettings(QSettings *settings);

private slots:
	void accept();
};

#endif

