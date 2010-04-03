#include <QtGui>

#include "backenddialog.h"

/*
class FileValidator : public QValidator {
public:
	FileValidator(QObject *parent=0) : QValidator(parent){}
	void fixup(QString &input) const {}
	State validate(QString &input, int &pos) const {
		qDebug(qPrintable("input="+input));
		if(input.isEmpty()) return Acceptable;
		QFileInfo fileInfo(input);
		if (fileInfo.exists() && fileInfo.isExecutable()) return Acceptable;
		qDebug(qPrintable("path="+fileInfo.path()));
		if (!fileInfo.path().isEmpty() && QFile::exists(fileInfo.path())) return Acceptable;
		return Invalid;
	}
};
*/

BackEndDialog::BackEndDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);

	mainWindow = qobject_cast<MainWindow*>(this->parent());
	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));

	label_Status_Orig = label_Status->text();
	label_Version_Orig = label_Version->text();
	//buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	/*
	FileValidator* fileValidator = new FileValidator(this);
	lineEdit_fileName->setValidator(fileValidator);
	*/
	
	//connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	//connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	//connect(lineEdit_fileName, SIGNAL(editingFinished()), this, SLOT(on_lineEdit_fileName_editingFinished()));

	lineEdit_fileName->setText(mainWindow->getBackEndFileName());
	refreshStatusAndVersion(true);
	
	isAccepted = false;
}

BackEndDialog::~BackEndDialog(){
	if (isAccepted){
		mainWindow->setBackEndFileName(lineEdit_fileName->text());
		mainWindow->setBackEndVersion(label_Version->text());
	}
}

void BackEndDialog::refreshStatusAndVersion(bool isInit){
	bool isStatusOk = false;
	QString status = "<b><font color=red>"+tr("invalid")+"</font></b>";
	QString version = "";
	QString backEndVersion = isInit ? mainWindow->getBackEndVersion() : mainWindow->findBackEndVersionByProcess(lineEdit_fileName->text());
	if (backEndVersion.isEmpty()){
		status = "<b><font color=red>"+tr("invalid (not found)")+"</font></b>";
	}else{
		status = "<b><font color=green>"+tr("ok")+"</font></b>";
		isStatusOk = true;
		version = backEndVersion;
	}
	label_Status->setText(label_Status_Orig.arg(status));
	label_Version->setText(label_Version_Orig.arg(version));
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isStatusOk);
}

void BackEndDialog::accept(){
	isAccepted = true;
	return QDialog::accept();
}

void BackEndDialog::on_toolButton_fileName_clicked(){
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select back end mp3gain executable file"),
													lineEdit_fileName->text(), QString());
	if (!fileName.isEmpty()){
		lineEdit_fileName->setText(fileName);
		refreshStatusAndVersion(false);
	}
}

void BackEndDialog::on_lineEdit_fileName_editingFinished(){
	refreshStatusAndVersion(false);
}
