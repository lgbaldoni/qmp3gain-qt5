#ifndef DISCLAIMERDIALOG_H
#define DISCLAIMERDIALOG_H

#include <QDialog>

#include "ui_disclaimerdialog.h"

class DisclaimerDialog : public QDialog, public Ui::DisclaimerDialog
{
    Q_OBJECT

public:
    DisclaimerDialog(QWidget *parent = 0);
};

#endif

