#include <QtGui>
#include <QPointer>

#include "logoptionsdialog.h"

/*
class FileValidator : public QValidator {
public:
	FileValidator(QObject *parent=0) : QValidator(parent){}
	void fixup(QString &input) const {}
	State validate(QString &input, int &pos) const {
		qDebug(qPrintable("input="+input));
		if(input.isEmpty()) return Acceptable;
		QFileInfo fileInfo(input);
		if (fileInfo.exists()) return Acceptable;
		qDebug(qPrintable("path="+fileInfo.path()));
		if (!fileInfo.path().isEmpty() && QFile::exists(fileInfo.path())) return Acceptable;
		return Invalid;
	}
};
*/

LogOptionsDialog::LogOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
	setupUi(this);
	
	/*
	FileValidator* fileValidator = new FileValidator(this);
	lineEdit_logFile->setValidator(fileValidator);
	*/
	
	//connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	//connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	mainWindow = qobject_cast<MainWindow*>(this->parent());
	setWindowTitle(windowTitle().arg(MainWindow::getAppTitle()));
	readSettings(mainWindow->getSettings());
	
	lineEdit_logFile->setText(mainWindow->getFileLog() ? mainWindow->getFileLog()->fileName() : QString());

	isAccepted = false;
}

LogOptionsDialog::~LogOptionsDialog(){
	if (isAccepted){
		writeSettings(mainWindow->getSettings());
	}
}

void LogOptionsDialog::readSettings(QSettings *settings){
	settings->beginGroup("logOptionsDialog");
	//lineEdit_logFile->setText(settings->value("lineEdit_logFile").toString());
	settings->endGroup();
}

void LogOptionsDialog::writeSettings(QSettings *settings){
	settings->beginGroup("logOptionsDialog");
	//settings->setValue("lineEdit_logFile", lineEdit_logFile->text());
	settings->endGroup();
}

void LogOptionsDialog::openFileDialog(QLineEdit *lineEdit){
	QFileDialog dialog(this, tr("Add log file(s)"), QString() /* directory */);
	QString fileName = lineEdit->text();
	if (QFile::exists(fileName)){
		dialog.selectFile(fileName);
	}
	QStringList filters;
	filters << "Log files (*.log)" << "Text files (*.txt)" << "Any files (*)";
	dialog.setNameFilters(filters);
	QStringList fileNames;
	if (dialog.exec()){
		fileNames = dialog.selectedFiles();
		if (!fileNames.isEmpty()){
			QString fileName = fileNames.at(0);
			lineEdit->setText(fileName);
		}
	}
}

void LogOptionsDialog::accept(){
	QPointer<QFile> fileLogActual = mainWindow->getFileLog();
	QPointer<QFile> fileLog;
	bool isToBeClosed = false;
	bool hasNewLog = false;
	if (!lineEdit_logFile->text().isEmpty()){
		fileLog = new QFile(lineEdit_logFile->text());
		if (!fileLog->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
			QMessageBox::critical(this, MainWindow::getAppTitle(),
					tr("Cannot write log file: %1").arg(fileLog->fileName()));
			delete fileLog;
			return;
		}
		else{
			isToBeClosed = true;
			hasNewLog = true;
		}
	}
	else{
		isToBeClosed = true;
	}
	if (isToBeClosed){
		if (fileLogActual){
			if (fileLogActual->isOpen())
				fileLogActual->close();
			delete fileLogActual;
		}
	}
	if (hasNewLog){
		mainWindow->setFileLog(fileLog);
	}
	isAccepted = true;
	return QDialog::accept();
}

void LogOptionsDialog::on_toolButton_logFile_clicked(){
	openFileDialog(lineEdit_logFile);
}

void LogOptionsDialog::on_buttonBox_clicked(QAbstractButton * button){
	if (button == buttonBox->button(QDialogButtonBox::Reset)){
		lineEdit_logFile->clear();
	}
	else if (button == buttonBox->button(QDialogButtonBox::RestoreDefaults)){
		QString fileName = QDir::homePath()+"/"+MainWindow::getAppTitle().toLower()+".log";
		lineEdit_logFile->setText(fileName);
	}
}
