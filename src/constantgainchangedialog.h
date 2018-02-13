#ifndef CONSTANTGAINCHANGEDIALOG_H
#define CONSTANTGAINCHANGEDIALOG_H

#include <QDialog>
#include <QSettings>
#include "ui_constantgainchangedialog.h"

class ConstantGainChangeDialog : public QDialog, public Ui::ConstantGainChangeDialog
{
    Q_OBJECT

public:
    ConstantGainChangeDialog(QWidget *parent = 0);
    ~ConstantGainChangeDialog();

private slots:
	void updateLineEdit_constGainChange(int value);
	void groupBox_onlyOneChannel_toggled(bool isOn);
};

#endif // CONSTANTGAINCHANGEDIALOG_H
