#ifndef DONATIONDIALOG_H
#define DONATIONDIALOG_H

#include <QDialog>

#include "ui_donationdialog.h"

class DonationDialog : public QDialog, public Ui::DonationDialog
{
    Q_OBJECT

public:
	DonationDialog(QWidget *parent = 0);

private slots:
	void on_donationButton_clicked();
};

#endif

