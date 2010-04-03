#include <QtGui>

#include "constantgainchangedialog.h"
#include "mainwindow.h"
#include "mymessagebox.h"

ConstantGainChangeDialog::ConstantGainChangeDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	connect(horizontalSlider_constGainChange, SIGNAL(valueChanged(int)), this, SLOT(updateLineEdit_constGainChange(int)));
	connect(groupBox_onlyOneChannel, SIGNAL(toggled(bool)), this, SLOT(groupBox_onlyOneChannel_toggled(bool)));

	updateLineEdit_constGainChange(horizontalSlider_constGainChange->value());
}

ConstantGainChangeDialog::~ConstantGainChangeDialog(){
}

void ConstantGainChangeDialog::updateLineEdit_constGainChange(int value){
	lineEdit_constGainChange->setText(QString("%1").arg((double)value*1.5, 0, 'f', 1));
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(value!=0);
}

void ConstantGainChangeDialog::groupBox_onlyOneChannel_toggled(bool isOn){
	if (isOn){
		MainWindow* mainWindow = qobject_cast<MainWindow*>(this->parent());
		mainWindow->getSettings()->beginGroup("logOptionsDialog");
		bool isConfirmSuppressed = mainWindow->getSettings()->value("groupBox_onlyOneChannel_ConfirmSuppressed", false).toBool();
		if (!isConfirmSuppressed){
			MyMessageBox::warning(this, MainWindow::getAppTitle()+QString(" - ")+tr("Warning"),
	                    tr("This function will only work if the mp3 is encoded as "
	                       "stereo or dual channel, NOT joint-stereo or mono."),
	                    tr("Don't show this warning again"),
	                    isConfirmSuppressed);
			if (isConfirmSuppressed){
				mainWindow->getSettings()->setValue("groupBox_onlyOneChannel_ConfirmSuppressed", true);
			}
		}
		mainWindow->getSettings()->endGroup();
	}
}
