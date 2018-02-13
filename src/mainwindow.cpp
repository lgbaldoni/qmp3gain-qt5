#include <QtGui>
#include <QtXml>
#include <QWidgetAction>
#include <QFileDialog>
#include <QAudioDeviceInfo>

#include "mainwindow.h"
#include "aboutdialog.h"
#include "advancedoptionsdialog.h"
#include "backenddialog.h"
#include "constantgainchangedialog.h"
#include "disclaimerdialog.h"
#include "logoptionsdialog.h"
#include "mymessagebox.h"

#ifndef APP_MAJOR_VER
#define APP_MAJOR_VER "?"
#endif
#ifndef APP_MINOR_VER
#define APP_MINOR_VER "?"
#endif
#ifndef APP_SUBMINOR_VER
#define APP_SUBMINOR_VER "?"
#endif
#ifndef APP_LASTCOMMIT_ID
#define APP_LASTCOMMIT_ID ""
#endif
#ifndef APP_LASTCOMMIT_DATE
#define APP_LASTCOMMIT_DATE ""
#endif

// init const static variables
const QString MainWindow::appTitle = "QMP3Gain";
const QString MainWindow::appVersion = QString() + APP_MAJOR_VER + "." + APP_MINOR_VER + "." + APP_SUBMINOR_VER;
const QString MainWindow::appLastCommitId = QString() + APP_LASTCOMMIT_ID; // "bcde360"
const QString MainWindow::appLastCommitDate = QString() + APP_LASTCOMMIT_DATE;  // "2010-05-10 00:08:20 +0200"
const QString MainWindow::backEndFixed = "mp3gain"; // only used if backEndFileName is empty, see this->getBackEnd()
const double MainWindow::defaultNormalTargetValue = 89.0;
const QString MainWindow::defaultLocale = "en_US";
const double MainWindow::DB = 20.0*log10(pow(2.0,0.25)); // 1 mp3Gain = ~1.5 dBGain
const QString MainWindow::donationUrl =
		"https://www.paypal.com/xclick/business=mp3gain@hotmail.com&item_name=MP3 Gain Donation&no_shipping=1&return=http://mp3gain.sourceforge.net/thanks.php";
const QString MainWindow::requiredBackEndVersion = "1.5.2";

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
{
	backEndFileName = ""; // if it is set then this one is used instead of backEndFixed
	isCancelled = false;
	lastAddedIndices = QModelIndexList();
	menuLanguageActionGroup = new QActionGroup(this);
	enabledGUI = false;

	appTranslator = new QTranslator(this);
	qApp->installTranslator(appTranslator);

	setupUi(this);

	// add context menu for tableView
	connect(tableView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenuForWidget(const QPoint &)));
	// modified default dB value
	connect(doubleSpinBox_targetNormalValue, SIGNAL(valueChanged(double)), this, SLOT(updateModelRowsByNewTargetNormalValue(double)));

	createLanguageMenu();
	
    // fill actions vector with checkable menu items
    {
		// Options
		actions << actionAlways_on_Top;
		actions << actionWork_on_Selected_files_only;
		actions << actionAdd_Subfolders;
		actions << actionPreserve_file_date_time;
		actions << actionNo_check_for_Layer_I_or_II;
		actions << actionDon_t_clip_when_doing_track_gain;
		// Options/Tags
		actions << actionIgnore_do_not_read_or_write_tags;
		actions << actionRe_calculate_do_not_read_tags;
		actions << actionDon_t_check_adding_files;
		actions << actionRemove_Tags_from_files;
		// Options/Logs
		actions << actionLogDock;
		actions << actionLogOpenAutomaticallyPanel;
		// Options/Logs/Output Type
		actions << actionLogError;
		actions << actionLogAnalysis;
		actions << actionLogChange;
		actions << actionLogBackend;
		actions << actionLogTrace;
		// Options/Logs/Timestamp
		actions << actionLogTimestampToFile;
		actions << actionLogTimestampToPanel;
		// Options/Toolbar
		actions << actionBig;
		actions << actionSmall;
		actions << actionText_only;
		actions << actionNone;
		// Options/Filename_Display
		actions << actionShow_Path_slash_File;
		actions << actionShow_File_only;
		actions << actionShow_Path_at_File;
		// Options
		actions << actionMinimize_to_tray;
		actions << actionBeep_when_finished;
	}
	
	// fill logOutputTypes with action - checkbox pairs
	logOutputTypes.insert(actionLogError, checkBox_logError);
	logOutputTypes.insert(actionLogAnalysis, checkBox_logAnalysis);
	logOutputTypes.insert(actionLogChange, checkBox_logChange);
	logOutputTypes.insert(actionLogBackend, checkBox_logBackend);

	// persist parameters
	settings = new QSettings("Zematix", appTitle);
	readSettings();
	
	// manage backEndVersion and backEndFileName
	backEndVersion = findBackEndVersionByProcess();
	if (backEndVersion.isEmpty() && !backEndFileName.isEmpty()){
		backEndFileName = "";
		backEndVersion = findBackEndVersionByProcess();
	}

	// to resize toolbar icons later
	iconDefaultSize = toolBar->iconSize();

	// this array is a must to enable translations of buttons of QDialogButtonBox
	static const char *QDialogButtonBox_strings[] = {
		// QDialogButtonBox buttons must be set in our translation file
		QT_TRANSLATE_NOOP("QDialogButtonBox", "OK"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Open"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Save"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Cancel"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Close"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Discard"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Apply"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Reset"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Restore Defaults"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Help"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Save All"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Yes to &All"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "&No"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "N&o to All"),

		QT_TRANSLATE_NOOP("QDialogButtonBox", "Abort"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Retry"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Ignore"),

		QT_TRANSLATE_NOOP("QDialogButtonBox", "Don't Save"),
		QT_TRANSLATE_NOOP("QDialogButtonBox", "Close without Saving")
	};

	// model and view columns for table containing files
	static const char * _modelHeaderLabels[] = {
		QT_TR_NOOP("Path/File"),
		QT_TR_NOOP("Path"),
		QT_TR_NOOP("File"),
		QT_TR_NOOP("Volume"),
		QT_TR_NOOP("Max Amplitude"), // hidden
		QT_TR_NOOP("clipping"),
		QT_TR_NOOP("Track Gain"),
		QT_TR_NOOP("dBGain"),  // hidden
		QT_TR_NOOP("clip(Track)"),
		QT_TR_NOOP("Max Noclip Gain"),
		QT_TR_NOOP("Album Volume"),
		QT_TR_NOOP("Album Max Amplitude"),  // hidden (in fact this field is never used)
		QT_TR_NOOP("Album Gain"),
		QT_TR_NOOP("Album dBGain"),  // hidden
		QT_TR_NOOP("clip(Album)"),
		0
	};
	modelHeaderLabels = _modelHeaderLabels;
	for (int i = 0; modelHeaderLabels[i]; ++i){
		modelHeaderList << modelHeaderLabels[i];
	}
	model = new QStandardItemModel;
	model->setHorizontalHeaderLabels(modelHeaderList);   // translated later
	tableView->setModel(model);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	int widthPathFile = tableView->columnWidth(modelHeaderList.indexOf("Path/File"))*2;
	int widthPath = tableView->columnWidth(modelHeaderList.indexOf("Path"))*2;
	int widthFile = tableView->columnWidth(modelHeaderList.indexOf("File"))*2;
	tableView->resizeColumnsToContents();
	tableView->setColumnWidth(modelHeaderList.indexOf("Path/File"), widthPathFile);
	tableView->setColumnWidth(modelHeaderList.indexOf("Path"), widthPath);
	tableView->setColumnWidth(modelHeaderList.indexOf("File"), widthFile);
	
	// store default widths of tableView
	QList<QVariant> widths; // QList<int>
	for (int i=0; i<tableView->model()->columnCount(); i++){
		widths.append(QVariant(tableView->columnWidth(i)));
	}
	tableView->setProperty("columnWidths", QVariant(widths));
	
	// initialize operationMap
	// contains operationId and a list with passes in percents
	operationMap.insert("add_file", QList<int>() << 100);
	operationMap.insert("add_file, analysis", QList<int>() << 30 << 70);
	operationMap.insert("add_folder", QList<int>() << 100);
	operationMap.insert("add_folder, analysis", QList<int>() << 30 << 70);
	operationMap.insert("track_analysis", QList<int>() << 100);
	operationMap.insert("album_analysis", QList<int>() << 80 << 20);
	operationMap.insert("max_no_clip_analysis", QList<int>() << 100);
	operationMap.insert("clear_analysis", QList<int>() << 100);
	operationMap.insert("track_gain", QList<int>() << 100);
	operationMap.insert("album_gain", QList<int>() << 100);
	operationMap.insert("constant_gain", QList<int>() << 100);
	operationMap.insert("max_no_clip_gain_for_each_file", QList<int>() << 50 << 50);
	operationMap.insert("max_no_clip_gain_for_album", QList<int>() << 50 << 50);
	operationMap.insert("undo_gain_changes", QList<int>() << 100);

	// init beepSound
	on_actionBeep_when_finished_toggled(settings->value("actionBeep_when_finished", false).toBool());

	createStatusBar();

	refreshUi();
	refreshGUI();
}

MainWindow::~MainWindow(){
	// writesettings() cannot be used here due to on_logDockWidget_visibilityChanged
	delete appTranslator;
	delete menuLanguageActionGroup;
	delete settings;
	delete model;
	delete beepSound;

	delete trayIcon;
	delete restoreTrayAction;
	delete quitTrayAction;

	delete actionPlay_mp3_file;
	delete mediaObject;
}

long MainWindow::getVersionNumber(const QString & versionString)
{
	if (versionString.isEmpty())
		return 0;

	long major = 0;
	long minor = 0;
	long subminor = 0;
	QRegExp rx("(?:^)(\\d+)(?:\\.)(\\d+)(?:\\.)(\\d+)(?:$)");
	int pos = rx.indexIn(versionString);
	if (pos > -1) {
		major = rx.cap(1).toInt();
		minor = rx.cap(2).toInt();
		subminor = rx.cap(3).toInt();
	}

	return major*1000000+minor*1000+subminor;
}

/*
bool MainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("Spreadsheet"),
						tr("The document has been modified.\nDo you want to save your changes?"),
                        QMessageBox::Yes | QMessageBox::No
                        | QMessageBox::Cancel);
		operationTime.restart();
        if (r == QMessageBox::Yes) {
            return true; //save();
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}
*/

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (true /*okToContinue()*/) {
		writeSettings(); // moving this call into the destructor is not an option
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::changeEvent (QEvent *event)
{
	QWidget *parentWidget = new QWidget(0, Qt::Window);
	bool isEventProcessed = false;

	switch (event->type()) {
		case QEvent::WindowStateChange: {
			/*
			QWindowStateChangeEvent *changeEvent = static_cast<QWindowStateChangeEvent*>(event);
			if (changeEvent->isOverride())
				break;
			Qt::WindowStates oldState = changeEvent->oldState();
			Qt::WindowStates newState = this->windowState();
			if (!(oldState & Qt::WindowMinimized) && (newState & Qt::WindowMinimized)){
			*/
			if (isMinimized()){
				if (actionMinimize_to_tray->isChecked()){
					if (!trayIcon)
						createTrayIcon();

					if (trayIcon){
						mainGeometry = saveGeometry();
						setParent(parentWidget, Qt::SubWindow); // removing from taskbar in Windows
						//hide();
						trayIcon->show();
						event->ignore();
						isEventProcessed = true;
					}
				}
			}
			break;
		}
		default:
			;
	}

	if (!isEventProcessed)
		event->accept();
}


/*
void MainWindow::showEvent(QShowEvent *event)
{
}
*/

void MainWindow::createStatusBar()
{
	messageLabel = new QLabel;
	messageLabel->setIndent(3);

	modelRowCountLabel = new QLabel(" 0 ");
	modelRowCountLabel->setAlignment(Qt::AlignHCenter);
	modelRowCountLabel->setMinimumSize(modelRowCountLabel->sizeHint());
	modelRowCountLabel->setToolTip(tr("Number of files in the list"));

	statusBar()->addWidget(messageLabel, 1);
	statusBar()->addWidget(modelRowCountLabel);

	connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
			this, SLOT(updateStatusBar()));
	connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
			this, SLOT(updateStatusBar()));
/*
	connect(spreadsheet, SIGNAL(currentCellChanged(int, int, int, int)),
			this, SLOT(updateStatusBar()));
	connect(spreadsheet, SIGNAL(modified()),
			this, SLOT(spreadsheetModified()));
*/
	updateStatusBar();
}

void MainWindow::updateStatusBar(){
	modelRowCountLabel->setText(QString(" %1 ").arg(model->rowCount()));
}

void MainWindow::updateStatusBar(const QString & msg){
	messageLabel->setText(msg);
	updateStatusBar();
}

void MainWindow::createTrayIcon()
{
	if (QSystemTrayIcon::isSystemTrayAvailable()){
		QIcon icon;
		icon.addFile(QString::fromUtf8(":/images/icon.png"), QSize(), QIcon::Normal, QIcon::Off);

		// create tray actions
		restoreTrayAction = new QAction(tr("&Restore"), this);
		connect(restoreTrayAction, SIGNAL(triggered()), this, SLOT(trayHide()));

		quitTrayAction = new QAction(tr("&Quit"), this);
		quitTrayAction->setShortcut(QKeySequence(tr("Ctrl+Q", "SystemTrayIcon|Quit")));
		connect(quitTrayAction, SIGNAL(triggered()), qApp, SLOT(quit()));

		// create tray menu
		QMenu *trayIconMenu = new QMenu(this);

		/*
		QLabel* label = new QLabel(QString("<b>"+this->appTitle)+"</b>");
		label->setAlignment( Qt::AlignLeft );
		*/

		QWidget *widget;
		QVBoxLayout *verticalLayout;
		QHBoxLayout *horizontalLayout;
		QLabel *label;
		QLabel *label_2;
		QSpacerItem *horizontalSpacer;

		widget = new QWidget();
		widget->setObjectName(QString::fromUtf8("widget"));
		//widget->setGeometry(QRect(100, 90, 103, 42));
		verticalLayout = new QVBoxLayout(widget);
		verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
		label = new QLabel(widget);
		label->setObjectName(QString::fromUtf8("label"));
		label->setPixmap(QPixmap(QString::fromUtf8(":/images/icon.png")));
		label->setMaximumSize(16, 16);
		label->setScaledContents(true);

		horizontalLayout->addWidget(label);

		label_2 = new QLabel(widget);
		label_2->setObjectName(QString::fromUtf8("label_2"));
		label_2->setText(QString("<b>"+this->appTitle)+"</b>");
		label_2->setAlignment(Qt::AlignCenter);

		horizontalLayout->addWidget(label_2);

		horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		horizontalLayout->addItem(horizontalSpacer);

		verticalLayout->addLayout(horizontalLayout);


		QWidgetAction* titleAction = new QWidgetAction(this);
		titleAction->setDefaultWidget(widget);

		trayIconMenu->addAction(titleAction);
		trayIconMenu->addSeparator();
		trayIconMenu->addAction(restoreTrayAction);
		trayIconMenu->addAction(quitTrayAction);

		// create tray icon
		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setContextMenu(trayIconMenu);

		trayIcon->setIcon(icon);
		trayIcon->setToolTip(tr("No operation running"));

		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
				this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	}
	else{
		writeLog(tr("Minimize to tray option is checked and tray icon should be created but system tray is unavailable"), LOGTYPE_TRACE);
	}

}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason){
	switch (reason) {
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::DoubleClick:
			trayHide();
			break;
		case QSystemTrayIcon::MiddleClick:
			trayShowMessage();
			break;
		default:
			;
	}
}

void MainWindow::trayShowMessage()
{
	if (QSystemTrayIcon::supportsMessages()){
		QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
		trayIcon->showMessage(appTitle, tr("Lossless volume modification of MP3 files"), icon, 5000);
	}
	else{
		writeLog(tr("System tray does not support balloon messages"), LOGTYPE_TRACE);
	}
}

void MainWindow::trayHide()
{
	setParent(0, Qt::Window);
	restoreGeometry(mainGeometry);
	showNormal();
	trayIcon->hide();
}

void MainWindow::playStateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
	QString playedFileName = mediaObject->currentSource().fileName();

	switch (newState) {
		case Phonon::ErrorState:
			writeLog(QString("Playing of %1 went into failure because \"%2\"").arg(playedFileName).arg(mediaObject->errorString()), LOGTYPE_ERROR);
			QMessageBox::critical(this, appTitle+" - "+tr("Fatal Error"),
				tr("Playing of %1 went into failure").arg(playedFileName));
			break;
		case Phonon::PlayingState:
				break;
		case Phonon::StoppedState:
				break;
		case Phonon::PausedState:
				break;
		case Phonon::BufferingState:
				break;
		default:
			;
	}
}

// contextmenu: Play mp3 file
void MainWindow::playMP3File(){
	try {
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		QStringList args;
		args << "-s" << "d"; // delete stored tag info (no other processing)
		args << getArgumentsByOptions();

		QModelIndex index = indices.last();
		int row = model->itemFromIndex(index)->row();
		//QStandardItem *item = model->itemFromIndex(index);
		QStandardItem *item = getItem(row, "Path/File");
		QString fileName = item->text();
		writeLog(fileName, LOGTYPE_TRACE);
		//fileName = QDir::toNativeSeparators(fileName);
		//writeLog(fileName, LOGTYPE_TRACE);

		//QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
		//writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		bool isToBeStopped = false;
		if (mediaObject){
			Phonon::State state = mediaObject->state();
			if (state && state==Phonon::PlayingState){
				QString playedFileName = mediaObject->currentSource().fileName();
				if (fileName==playedFileName){
					isToBeStopped = true;
				}
			}
			delete mediaObject;
		}
		if (!isToBeStopped) {
			mediaObject = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(fileName));
			connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
						this, SLOT(playStateChanged(Phonon::State,Phonon::State)));
			mediaObject->play();
		}

		throw(0);
	}
	catch (int e){
	}
}

void MainWindow::on_cancelButton_clicked()
{
	isCancelled = true;
	if (donationView){
		delete donationView;
	}
	writeLog(QString("isCancelled := true"), LOGTYPE_TRACE);
}

void MainWindow::on_clearLogButton_clicked()
{
	try {
		bool isConfirmSuppressed = settings->value("clearLogs_ConfirmSuppressed", false).toBool();
		if (!isConfirmSuppressed){
			int r = MyMessageBox::question(this, appTitle+" - "+tr("Clear Logs?"),
						tr("This will clear all log results.\n"
						   "Are you sure?"),
						tr("Don't ask me again"),
						isConfirmSuppressed,
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::No);
			operationTime.restart();
			if (r == QMessageBox::No) {
				throw(0);
			}
			if (isConfirmSuppressed){
				settings->setValue("clearLogs_ConfirmSuppressed", true);
			}
		}

		this->logPlainTextEdit->clear();
		throw(0);
	}
	catch (int e){
	}
}

void MainWindow::on_logDockWidget_visibilityChanged(bool visible)
{
	actionLogDock->setChecked(visible);
}

QDir MainWindow::directoryOf(const QString &subdir)
{
	QDir dir(QApplication::applicationDirPath());
	//writeLog(QApplication::applicationDirPath(), LOGTYPE_TRACE);

#if defined(Q_OS_WIN)
	//if (dir.dirName().toLower() == "debug" || dir.dirName().toLower() == "release")
	if (dir.dirName().toLower() == "bin")
		dir.cdUp();
#elif defined(Q_OS_MAC)
	if (dir.dirName() == "MacOS") {
		dir.cdUp();
		dir.cdUp();
		dir.cdUp();
	}
#else //if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
	if (dir.absolutePath().toLower() == "/usr/bin")
		dir.cd("/usr/share/qmp3gain");
	else if (dir.dirName().toLower() == "bin")
		dir.cdUp();
#endif
	dir.cd(subdir);
	return dir;
}

void MainWindow::switchLanguage(QAction *action)
{
	QString locale("");

	if (!action) {
		locale = settings->value("locale", QLocale::system().name()).toString();
	}else{
		locale = action->data().toString();
	}

	QString qmPath = directoryOf("translations").absolutePath();
	QString qmFile = appTitle.toLower() + QString("_") + locale;
	QString qmPathFile = qmPath+"/"+qmFile+".qm";

	bool isLoaded = appTranslator->load(qmFile, qmPath);
	if (isLoaded) {
		writeLog(tr("Translation file %1 is loaded").arg(qmPathFile), LOGTYPE_TRACE);
		settings->setValue("locale", locale);
		// search for the action belongs to our locale
		foreach(QAction *a, menuLanguageActionGroup->actions()) {
			if (a->data().toString() == locale) {
				a->setChecked(true);
				writeLog(tr("%1 locale is used").arg(locale), LOGTYPE_TRACE);
				break;
			}
		}
	}
	else{
		if (locale==defaultLocale)
			writeLog(tr("No translation file is necessary"), LOGTYPE_TRACE);
		else
			writeLog(tr("Translation file %1 cannot be found, %2 locale is refused").arg(qmPathFile).arg(locale), LOGTYPE_ERROR);

		settings->setValue("locale", defaultLocale);
		actionDefault_Language->setChecked(true);
		writeLog(tr("Default %1 locale is used").arg(defaultLocale), LOGTYPE_TRACE);
	}

	retranslateUi(this);
	refreshUi();
	refreshGUI();
}

void MainWindow::createLanguageMenu()
{
	// fist menu item: actionDefault_Language "Original (English)"
	actionDefault_Language->setData(defaultLocale);
	menuLanguageActionGroup->addAction(actionDefault_Language);

	connect(menuLanguageActionGroup, SIGNAL(triggered(QAction *)),
	        this, SLOT(switchLanguage(QAction *)));

	QDir qmDir = directoryOf("translations");
	QStringList fileNames = qmDir.entryList(QStringList(appTitle.toLower()+"_*.qm"));

	foreach (QString fileName, fileNames) {
		QString locale = fileName;
		locale.remove(0, locale.indexOf('_') + 1);       // remove appTitle_ from the start
		locale.chop(3);    // remove ".qm" from the end

		QTranslator translator;
		translator.load(fileName, qmDir.absolutePath());
		QString language = translator.translate("MainWindow", "Language");

		QAction *action = new QAction(language, this);
		action->setCheckable(true);
		action->setData(locale);

		menuLanguage->addAction(action);
		menuLanguageActionGroup->addAction(action);
	}
}

void MainWindow::writeSettings()
{
	settings->setValue("geometry", saveGeometry());
	settings->setValue("state", saveState());
	//settings->setValue("splitter", splitter->saveState());

	settings->setValue("doubleSpinBox_targetNormalValue", doubleSpinBox_targetNormalValue->value());
	//settings.setValue("recentFiles", recentFiles);
	//settings.setValue("showGrid", showGridAction->isChecked());
	//settings.setValue("autoRecalc", autoRecalcAction->isChecked());

	// action elements
	foreach (QAction* action, actions) {
		settings->setValue(action->objectName(), action->isChecked());
	}

	if (lastAddedFolder==".")
		settings->remove("lastAddedFolder");
	else
		settings->setValue("lastAddedFolder", lastAddedFolder);

	if (!backEndFileName.isEmpty())
		settings->setValue("backEndFileName", backEndFileName);
	else
		settings->remove("backEndFileName");

	if (fileLog && !fileLog->fileName().isEmpty())
		settings->setValue("logFileName", fileLog->fileName());
	else
		settings->remove("logFileName");

	if (fileLog){
		if (fileLog->isOpen())
			fileLog->close();
		delete fileLog;
	}
}

void MainWindow::readSettings()
{
	restoreGeometry(settings->value("geometry").toByteArray());
	restoreState(settings->value("state").toByteArray());
	//splitter->restoreState(settings->value("splitter").toByteArray());

	doubleSpinBox_targetNormalValue->setValue(settings->value("doubleSpinBox_targetNormalValue", defaultNormalTargetValue).toDouble());

	//recentFiles = settings.value("recentFiles").toStringList();
	//updateRecentFileActions();

	// action elements
	foreach (QAction* action, actions) {
		action->setChecked(settings->value(action->objectName(), action->isChecked()).toBool());
	}

	// log checkboxes
	foreach(QAction *action, logOutputTypes.keys()) {
		QCheckBox *cb = logOutputTypes.value(action);
		cb->setChecked(settings->value(action->objectName(), action->isChecked()).toBool());
	}

	lastAddedFolder = settings->value("lastAddedFolder", ".").toString();
	QFileInfo fi(lastAddedFolder);
	if (!fi.exists()) lastAddedFolder = ".";

	backEndFileName = settings->value("backEndFileName", "").toString();

	QString logFileName = settings->value("logFileName", "").toString();
	if (!logFileName.isEmpty()){
		fileLog = new QFile(logFileName);
		if (!fileLog->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
			QMessageBox::critical(this, appTitle,
					tr("Cannot write log file: %1").arg(fileLog->fileName()));
			operationTime.restart();
			delete fileLog;
		}
	}
}

void MainWindow::refreshUi(){
	if (windowTitle().contains("%1")){
		QString title = appTitle;
		bool isDevelopmentRelease = !appLastCommitId.isEmpty() && !appLastCommitDate.isEmpty();
		if (isDevelopmentRelease){
			title += " "+appVersion;
			title += "."+appLastCommitId+" ("+appLastCommitDate+")";
		}
		setWindowTitle(windowTitle().arg(title));
	}
	
	// explanation: labelTargetNormalValue->text() = "dB (default %1)"
	labelTargetNormalValue->setText(QString(labelTargetNormalValue->text()).arg(defaultNormalTargetValue, 0, 'f', 1));
	
	QStringList modelTranslatedHeaderList;
	for ( int i = 0; modelHeaderLabels[i]; ++i ) {
		modelTranslatedHeaderList << tr(modelHeaderLabels[i]);
	}
	model->setHorizontalHeaderLabels(modelTranslatedHeaderList);
	//tableView->resizeColumnsToContents();
}

void MainWindow::enableGUI(){
	// enable complete GUI except cancelButton and tableView
	menubar->setEnabled(true);
	toolBar->setEnabled(true);
	doubleSpinBox_targetNormalValue->setEnabled(true);
	cancelButton->setEnabled(false);
	isCancelled = false;
	isPopupErrorSuppressed = QVariant();
	isOpenLogPanelQuestionSuppressed = QVariant();
	setProgress(QVariant(0), QVariant(0));
	updateStatusBar("");

	//logDockWidget
	clearLogButton->setEnabled(true);
	groupBox_logCheckboxes->setEnabled(true);

	int spinBox_beepAfter = settings->value("advancedOptionsDialog/spinBox_beepAfter", 0).toInt();
	int elapsedTime = operationTime.elapsed()/1000; // ms -> second
	if (actionBeep_when_finished->isChecked() && elapsedTime>=spinBox_beepAfter){
		if (beepSound)
			beepSound->play();
		else
			QApplication::beep();
	}

	this->enabledGUI = true;
}

void MainWindow::disableGUI(){
	// disable complete GUI except cancelButton and tableView
	menubar->setEnabled(false);
	toolBar->setEnabled(false);
	doubleSpinBox_targetNormalValue->setEnabled(false);
	cancelButton->setEnabled(true);
	isCancelled = false;
	isPopupErrorSuppressed = QVariant();
	isOpenLogPanelQuestionSuppressed = QVariant();
	setProgress(QVariant(0), QVariant(0));

	//logDockWidget
	clearLogButton->setEnabled(false);
	groupBox_logCheckboxes->setEnabled(false);

	// stop played mp3
	delete mediaObject;

	this->enabledGUI = false;
	operationTime.start();
}

void MainWindow::refreshGUI() {
	// show/hide some columns
	if (actionShow_Path_slash_File->isChecked()) {
		tableView->setColumnHidden(modelHeaderList.indexOf("Path/File"), false);
		tableView->setColumnHidden(modelHeaderList.indexOf("Path"), true);
		tableView->setColumnHidden(modelHeaderList.indexOf("File"), true);
	} else if (actionShow_File_only->isChecked()) {
		tableView->setColumnHidden(modelHeaderList.indexOf("Path/File"), true);
		tableView->setColumnHidden(modelHeaderList.indexOf("Path"), true);
		tableView->setColumnHidden(modelHeaderList.indexOf("File"), false);
	} else if (actionShow_Path_at_File->isChecked()) {
		tableView->setColumnHidden(modelHeaderList.indexOf("Path/File"), true);
		tableView->setColumnHidden(modelHeaderList.indexOf("Path"), false);
		tableView->setColumnHidden(modelHeaderList.indexOf("File"), false);
	}
	// these columns are always hidden
	bool checkBox_ShowHiddenFields = settings->value("advancedOptionsDialog/checkBox_ShowHiddenFields", false).toBool();
	tableView->setColumnHidden(modelHeaderList.indexOf("Max Amplitude"), !checkBox_ShowHiddenFields);
	tableView->setColumnHidden(modelHeaderList.indexOf("Album Max Amplitude"), !checkBox_ShowHiddenFields);
	tableView->setColumnHidden(modelHeaderList.indexOf("dBGain"), !checkBox_ShowHiddenFields);
	tableView->setColumnHidden(modelHeaderList.indexOf("Album dBGain"), !checkBox_ShowHiddenFields);

	// toolbar visualization
	if (actionBig->isChecked()) {
		toolBar->setVisible(true);
		toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		toolBar->setIconSize(iconDefaultSize);
	}else if (actionSmall->isChecked()) {
		toolBar->setVisible(true);
		toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		toolBar->setIconSize(iconDefaultSize/2.0);
	}else if (actionText_only->isChecked()) {
		toolBar->setVisible(true);
		toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
	}else if (actionNone->isChecked()) {
		toolBar->setVisible(false);
	}

	// log frame
	logDockWidget->setVisible(actionLogDock->isChecked());

	settings->beginGroup("advancedOptionsDialog");

	bool checkBox_Maximizing = settings->value("checkBox_Maximizing", false).toBool();
	tableView->setColumnHidden(modelHeaderList.indexOf("Max Noclip Gain"), !checkBox_Maximizing);

	bool checkBox_ShowNoFileProgress = settings->value("checkBox_ShowNoFileProgress", false).toBool();
	label_FileProgress->setVisible(!checkBox_ShowNoFileProgress);
	progressBar_File->setVisible(!checkBox_ShowNoFileProgress);

	bool isLogBackendEnabled = settings->value("horizontalSlider_logBackendDepth", 0).toInt() > 0;
	checkBox_logBackend->setVisible(isLogBackendEnabled);

	bool isLogTraceEnabled = settings->value("horizontalSlider_logTraceDepth", 0).toInt() > 0;
	checkBox_logTrace->setVisible(isLogTraceEnabled);

	settings->endGroup();

	refreshMenu();
}

void MainWindow::refreshMenu(){
	bool isBackEndAvailable = this->isBackEndAvailable();
	bool listEmpty = model->rowCount()==0;

	// some options might be not used
	if (!QSystemTrayIcon::isSystemTrayAvailable())
		actionMinimize_to_tray->setEnabled(false);

	actionLoad_Analysis_results->setEnabled(isBackEndAvailable);
	actionSave_Analysis_results->setEnabled(!listEmpty);
	actionAdd_Files->setEnabled(isBackEndAvailable);
	actionAdd_Folder->setEnabled(isBackEndAvailable);
	actionSelect_All_Files->setEnabled(!listEmpty);
	actionSelect_No_Files->setEnabled(!listEmpty);
	actionInvert_selection->setEnabled(!listEmpty);
	actionClear_Selected_Files->setEnabled(!listEmpty);
	actionClear_All_files->setEnabled(!listEmpty);
	actionTrack_Analysis->setEnabled(isBackEndAvailable && !listEmpty);
	actionAlbum_Analysis->setEnabled(isBackEndAvailable && !listEmpty);
	actionClear_Analysis->setEnabled(isBackEndAvailable && !listEmpty);
	actionTrack_Gain->setEnabled(isBackEndAvailable && !listEmpty);
	actionAlbum_Gain->setEnabled(isBackEndAvailable && !listEmpty);
	actionConstant_Gain->setEnabled(isBackEndAvailable && !listEmpty);
	actionUndo_Gain_changes->setEnabled(isBackEndAvailable && !listEmpty);
	actionRemove_Tags_from_files->setEnabled(isBackEndAvailable && !listEmpty);

	// some items are visible only if Options/Advanced/Maximizing checkbox is on
	bool maximizing = settings->value("advancedOptionsDialog/checkBox_Maximizing", false).toBool();
	actionMax_No_clip_Analysis->setEnabled(isBackEndAvailable && !listEmpty && maximizing);
	actionMax_No_clip_Analysis->setVisible(isBackEndAvailable && maximizing);
	actionMax_No_clip_Gain_for_Each_file->setEnabled(isBackEndAvailable && !listEmpty && maximizing);
	actionMax_No_clip_Gain_for_Each_file->setVisible(isBackEndAvailable && maximizing);
	actionMax_No_clip_Gain_for_Album->setEnabled(isBackEndAvailable && !listEmpty && maximizing);
	actionMax_No_clip_Gain_for_Album->setVisible(isBackEndAvailable && maximizing);

	bool isLogBackendEnabled = settings->value("advancedOptionsDialog/horizontalSlider_logBackendDepth", 0).toInt() > 0;
	actionLogBackend->setVisible(isLogBackendEnabled);
	bool isLogTraceEnabled = settings->value("advancedOptionsDialog/horizontalSlider_logTraceDepth", 0).toInt() > 0;
	actionLogTrace->setVisible(isLogTraceEnabled);
}

QStringList MainWindow::getArgumentsByOptions(){
	QStringList args;

	// output is a database-friendly tab-delimited list
	args << "-o";

	// ignore clipping warning when applying gain
	args << "-c";

	if (doubleSpinBox_targetNormalValue->value()!=defaultNormalTargetValue){
		// -d <n> - modify suggested dB gain by floating-point n
		args << "-d" << QString("%1").arg(doubleSpinBox_targetNormalValue->value()-defaultNormalTargetValue, 0, 'f', 1);
	}
	if (actionPreserve_file_date_time->isChecked()){
		// p - Preserve original file timestamp
		args << "-p";
	}
	if (actionNo_check_for_Layer_I_or_II->isChecked()){
		// -f - Force mp3gain to assume input file is an MPEG 2 Layer III file
		//      (i.e. don't check for mis-named Layer I or Layer II files)
		args << "-f";
	}
	if (actionDon_t_clip_when_doing_track_gain->isChecked()){
		// -k - automatically lower Track/Album gain to not clip audio
		args << "-k";
	}
	if (actionIgnore_do_not_read_or_write_tags->isChecked()){
		// -s s - skip (ignore) stored tag info (do not read or write tags)
		args << "-s" << "s";
	}
	if (actionRe_calculate_do_not_read_tags->isChecked()){
		// -s r - force re-calculation (do not read tag info)
		args << "-s" << "r";
	}
	if (!settings->value("advancedOptionsDialog/checkBox_UseNoTempFiles", false).toBool()){
		// -t - writes modified data to temp file, then deletes original
		//      instead of modifying bytes in original file
		args << "-t";
	}

	return args;
}

void MainWindow::setProgress(QVariant progressFile, QVariant progressTotal){
	if (!progressFile.isNull()){
		bool isValid;
		double value = progressFile.toDouble(&isValid);
		if (isValid){
			progressBar_File->setValue(value);
			writeLog(QString("progressFile: %1%").arg(progressBar_File->value()), LOGTYPE_TRACE);
		}
	}
	if (!progressTotal.isNull()){
		bool isValid;
		double value = progressTotal.toDouble(&isValid);
		if (isValid){
			progressBar_Total->setValue(value);
			if (trayIcon){
				if (value!=0)
					trayIcon->setToolTip(tr("Work in progress: %1%").arg(progressBar_Total->value()));
				else
					trayIcon->setToolTip(tr("No operation running"));
			}
			writeLog(QString("progressTotal: %1%").arg(progressBar_Total->value()), LOGTYPE_TRACE);
		}
	}else{
		if (trayIcon)
			trayIcon->setToolTip(tr("No operation running"));
	}
}

int MainWindow::getMP3FilesByFolder(const QString & dir, const int level, const double passSlice){
	try {
		QDir qdir(dir);
		QFileInfoList fileInfoList = qdir.entryInfoList ( QStringList() << "*.mp3",  QDir::Files, QDir::NoSort );
		QFileInfoList dirInfoList;
		if (actionAdd_Subfolders->isChecked()){
			dirInfoList = qdir.entryInfoList ( QStringList(),  QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::NoSort );
		}
		double startProgress = progressBar_Total->doubleValue();

		for (int i = 0; i < fileInfoList.size(); ++i) {
			if (level==0){
				setProgress(QVariant(),
							QVariant(startProgress+passSlice*(((double)(i+1))/(fileInfoList.size()+(actionAdd_Subfolders->isChecked() ? dirInfoList.size() : 0)))));
				qApp->processEvents(QEventLoop::AllEvents);
				if (isCancelled) throw(-1);
			}
			QFileInfo fi(fileInfoList.at(i));
			// check that actual filename is already stored in model
			QList<QStandardItem *> found = model->findItems(fi.absoluteFilePath(), Qt::MatchExactly, 0);
			if (found.count()>0) continue;
			QStandardItem *item0 = new QStandardItem(fi.absoluteFilePath());
			QStandardItem *item1 = new QStandardItem(fi.absolutePath());
			QStandardItem *item2 = new QStandardItem(fi.fileName());
			model->appendRow(QList<QStandardItem *>() << item0 << item1 << item2);
			QModelIndex index = model->indexFromItem(item0);
			if (index.isValid()){
				lastAddedIndices.append(index);
			}
		}

		if (actionAdd_Subfolders->isChecked()){
			for (int i = 0; i < dirInfoList.size(); ++i) {
				if (level==0){
					setProgress(QVariant(),
								QVariant(startProgress+passSlice*(((double)(fileInfoList.size()+i+1))/(fileInfoList.size()+dirInfoList.size()))));
					qApp->processEvents(QEventLoop::AllEvents);
					if (isCancelled) throw(-1);
				}
				QFileInfo fi(dirInfoList.at(i));
				getMP3FilesByFolder(fi.absoluteFilePath(), level+1);
			}
		}
		throw(0);
	}
	catch (int e){
		return e;
	}
	return 0;
}

QString MainWindow::getItemText(int row, const QString & column) {
	QStandardItem *item = getItem(row, column);
	return item ? item->text() : QString();
}

QVariant MainWindow::getItemValue(int row, const QString & column) {
	QStandardItem *item = getItem(row, column);
	return item ? item->data() : QVariant();
}

void MainWindow::setItem(int row, QString column, QVariant value) {
	QStandardItem *item = getItem(row, column);

	if (value.type()==QVariant::Double){
		QString valueStr = value.isNull() ? QString() : QString("%1").arg(value.toDouble(), 0, 'f', 1);
		if (item){
			if (!value.isNull()){
				item->setText(valueStr);
				item->setData(value);
			}else{
				item->setData(0); // before item delete to enable automatic model/view refresh
				delete item;
			}
		}else{
			if (!value.isNull()){
				item = new QStandardItem(valueStr);
				item->setData(value);
				model->setItem(row, modelHeaderList.indexOf(column), item);
			}else{
				// no creation
			}
		}
	}else if (value.type()==QVariant::Bool){
		QString trueStr = tr("Y", "Yes flag in some clipping fields of the file list");
		QString falseStr = tr("", "No flag in some clipping fields of the file list");
		if (item){
			if (!value.isNull()){
				item->setText(value.toBool() ? trueStr : falseStr);
				item->setData(value);
			}else{
				item->setData(0); // before item delete to enable automatic model/view refresh
				delete item;
			}
		}else{
			if (!value.isNull()){
				item = new QStandardItem(value.toBool() ? trueStr : falseStr);
				item->setData(value);
				model->setItem(row, modelHeaderList.indexOf(column), item);
			}else{
				// no creation
			}
		}
	}else{
		return; // error
	}
}

void MainWindow::writeLog(const QString & msg, LogType logType, int level, LogOption logOption) {
	bool isLogChecked = false;
	int storedLevel;
	QString styleSheet;
	QString txtMsg = msg;
	QString htmlMsg = msg;

	switch (logType) {
		case LOGTYPE_ERROR:
			styleSheet = this->checkBox_logError->styleSheet();
			if (this->checkBox_logError->isChecked())
				isLogChecked = true;
			break;
		case LOGTYPE_ANALYSIS:
			styleSheet = this->checkBox_logAnalysis->styleSheet();
			if (this->checkBox_logAnalysis->isChecked())
				isLogChecked = true;
			break;
		case LOGTYPE_CHANGE:
			styleSheet = this->checkBox_logChange->styleSheet();
			if (this->checkBox_logChange->isChecked())
				isLogChecked = true;
			break;
		case LOGTYPE_BACKEND:
			storedLevel = settings->value("advancedOptionsDialog/horizontalSlider_logBackendDepth", 0).toInt();
			if (storedLevel >= level){
				styleSheet = this->checkBox_logBackend->styleSheet();
				if (this->checkBox_logBackend->isChecked())
					isLogChecked = true;
			}
			break;
		case LOGTYPE_TRACE:
			storedLevel = settings->value("advancedOptionsDialog/horizontalSlider_logTraceDepth", 0).toInt();
			if (storedLevel >= level){
				styleSheet = this->checkBox_logTrace->styleSheet();
				if (this->checkBox_logTrace->isChecked())
					isLogChecked = true;
			}
			break;
		default:
			break;
	}
	if (isLogChecked) {
		if (actionLogTimestampToFile->isChecked()){
			txtMsg = QString("%1\t%2").arg(QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate /*QString("dd/MM/yyyy hh:mm:ss")*/)).arg(msg);
		}
		if (actionLogTimestampToPanel->isChecked()){
			htmlMsg = QString("%1\t%2").arg(QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate /*QString("dd/MM/yyyy hh:mm:ss")*/)).arg(msg);
		}

		if (logOption & LOGOPTION_BOLD){
			htmlMsg = QString("<b>%1</b>").arg(htmlMsg);
		}

		QRegExp rx("(?:color: *)(\\w+)");
		QString colorName;
		if (rx.indexIn(styleSheet) > -1) {
			colorName = rx.cap(1);
		}
		if (!colorName.isEmpty())
			htmlMsg = QString("<font color=\"%1\">%2</font>").arg(colorName).arg(htmlMsg);

		if (fileLog && fileLog->isOpen()){
			QTextStream out(fileLog);
			out << txtMsg << "\n";
		}
		this->logPlainTextEdit->appendHtml(htmlMsg);
	}
}

void MainWindow::updateModelRowByAnalysisTrack(QString fileName, int mp3Gain, double dBGain, double maxAmplitude, bool maxNoclipGain, bool isLog){
	QList<QStandardItem *> found = model->findItems(fileName, Qt::MatchExactly, 0);
	if (found.count()==0) return;
	QModelIndex modelIndex = model->item(found[0]->row())->index();
	updateModelRowByAnalysisTrack(modelIndex, mp3Gain, dBGain, maxAmplitude, maxNoclipGain, isLog);
}

void MainWindow::updateModelRowByAnalysisTrack(QModelIndex modelIndex, int mp3Gain, double dBGain, double maxAmplitude, bool maxNoclipGain, bool isLog){
	int row = modelIndex.row();
	double gainValue = mp3Gain*DB; // dBGain
	QStandardItem *item = 0;

	if (!maxNoclipGain){
		setItem(row, "Volume", QVariant(doubleSpinBox_targetNormalValue->value()-dBGain));
	}

	setItem(row, "Max Amplitude", QVariant(maxAmplitude));
	bool clipping = maxAmplitude>32767.0;
	setItem(row, "clipping", QVariant(clipping));

	if (!maxNoclipGain){
		setItem(row, "Track Gain", QVariant(gainValue));
		setItem(row, "dBGain", QVariant(dBGain));

		bool clippingTrack = maxAmplitude*pow(2.0, round(gainValue/DB)/4.0)>32767.0;
		setItem(row, "clip(Track)", QVariant(clippingTrack));
	}

	int mp3GainNoClip = (int)floor((15.0-log10(maxAmplitude)/log10(2.0))*4.0);
	setItem(row, "Max Noclip Gain", QVariant(mp3GainNoClip*DB));

	// finally set line color to red if clipping is set
	QStringList columns = QStringList() << "Path/File" << "Path" << "File";
	foreach (QString element, columns){
		QStandardItem *item = getItem(row, element);
		item->setForeground(QBrush(QColor(0,0,0)));
	}
	item = getItem(row, "clipping");
	QStandardItem *item1 = getItem(row, "clip(Track)");
	if ((item && item->data().toBool()) || (item1 && item1->data().toBool())){
		if (item) item->setForeground(QBrush(QColor(255,0,0)));
		if (item1) item1->setForeground(QBrush(QColor(255,0,0)));
		foreach (QString element, columns){
			QStandardItem *item = getItem(row, element);
			item->setForeground(QBrush(QColor(255,0,0)));
		}
	}

	if (isLog) {
		QString trace;
		trace += QString("%1\t").arg(getItemText(row, "Path/File"));
		trace += QString("TrackdB: %1\t").arg(getItemValue(row, "dBGain").toString());
		trace += QString("MaxAmp: %1").arg(getItemValue(row, "Max Amplitude").toString());
		writeLog(trace, LOGTYPE_ANALYSIS);
	}
}

void MainWindow::updateModelRowsByAnalysisAlbum(bool isAlbum, QString fileName, int mp3Gain, double dBGain, /*double*/ QVariant maxAmplitude, bool isLog){
	QList<QStandardItem *> found = model->findItems(fileName, Qt::MatchExactly, 0);
	if (found.count()==0) return;
	QModelIndex modelIndex = model->item(found[0]->row())->index();
	updateModelRowsByAnalysisAlbum(isAlbum, QModelIndexList() << modelIndex, mp3Gain, dBGain, maxAmplitude, isLog);
}

void MainWindow::updateModelRowsByAnalysisAlbum(bool isAlbum, QModelIndexList indices, int mp3Gain, double dBGain, /*double*/ QVariant maxAmplitude, bool isLog){
	foreach(QModelIndex index, indices) {
		int row = model->itemFromIndex(index)->row();

		if (isAlbum){
			setItem(row, "Album Volume", QVariant(doubleSpinBox_targetNormalValue->value()-dBGain));
			setItem(row, "Album Max Amplitude", maxAmplitude);

			double gainValue = mp3Gain*DB; // dBGain
			setItem(row, "Album Gain", QVariant(gainValue));
			setItem(row, "Album dBGain", QVariant(dBGain));

			// track's max amplitude must be checked, not the album's one
			QStandardItem *item = getItem(row, "Max Amplitude");
			double trackMaxAmplitude = item->data().toDouble(); // get Max Amplitude value
			bool clippingAlbum = trackMaxAmplitude*pow(2.0, round(gainValue/DB)/4.0)>32767.0;
			setItem(row, "clip(Album)", QVariant(clippingAlbum));
		}

		// set line color to red if clipping is set
		QStringList columns = QStringList() << "Path/File" << "Path" << "File";
		foreach (QString element, columns){
			QStandardItem *item = getItem(row, element);
			item->setForeground(QBrush(QColor(0,0,0)));
		}
		QStandardItem *item = getItem(row, "clipping");
		QStandardItem *item1 = getItem(row, "clip(Track)");
		QStandardItem *item2 = getItem(row, "clip(Album)");
		if ((item && item->data().toBool()) || (item1 && item1->data().toBool()) || (item2 && item2->data().toBool())){
			if (item) item->setForeground(QBrush(QColor(255,0,0)));
			if (item1) item1->setForeground(QBrush(QColor(255,0,0)));
			if (item2) item2->setForeground(QBrush(QColor(255,0,0)));
			foreach (QString element, columns){
				QStandardItem *item = getItem(row, element);
				item->setForeground(QBrush(QColor(255,0,0)));
			}
		}

		if (isLog) {
			QString trace;
			trace += QString("%1\t").arg(getItemText(row, "Path/File"));
			trace += QString("AlbumdB: %1").arg(getItemValue(row, "Album dBGain").toString());
			//trace += QString("AlbumMaxAmp: %1").arg(getItemValue(row, "Album Max Amplitude").toString());
			writeLog(trace, LOGTYPE_ANALYSIS);
		}
	}
}

QModelIndexList MainWindow::getModelIndices(){
	QModelIndexList indices;
	bool isFromContextMenu = sender() && sender()->property("calledFromContextMenu").toBool();

	if (isFromContextMenu || actionWork_on_Selected_files_only->isChecked()){
		QItemSelectionModel *selectionModel = tableView->selectionModel();
		indices = selectionModel->selectedRows(modelHeaderList.indexOf("Path/File"));
	}else{
		for (int row=0; row<model->rowCount(); row++){
			indices << getItemIndex(row, "Path/File");
		}
	}
	return indices;
}

MainWindow::ErrType MainWindow::hasError(const QString & input) {
	ErrType errType = ERRTYPE_NONE;
	QString logMsg, popupMsg;

	try {
		QRegExp rx;

		if (input.contains("Error analyzing further samples (max time reached)")) {
			logMsg = tr("Error analyzing further samples (max time reached)");
			popupMsg = logMsg;
			throw(ERRTYPE_ANALYSING_MAX_TIME_REACHED);
		}

		rx.setPattern("Cancelled processing of (.*)");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Cancelled processing of %1").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_CANCELLED_PROCESSING);
		}

		// QRegExp("Cancelled processing\.\n(.*) is probably corrupted now\."
		if (input == "Cancelled processing.") {
			throw(ERRTYPE_SUPPRESSED);
		}
		rx.setPattern("(.*) is probably corrupted now\\.");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Cancelled processing.") +
					 " " +
					 tr("%1 is probably corrupted now.").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_CANCELLED_PROCESSING_CORRUPT);
		}

		rx.setPattern("(.*): Can't adjust single channel for mono or joint stereo");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("%1 is not a stereo or dual-channel mp3").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_CANNOT_ADJUST_SINGLE_CHANNEL);
		}

		rx.setPattern("Can't find any valid MP3 frames in file (.*)");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Error while analyzing: %1").arg(tr("Can't find any valid MP3 frames in file %1").arg(fileName));
			popupMsg = logMsg;
			throw(ERRTYPE_CANNOT_FIND_MP3_FRAME);
		}

		rx.setPattern("Can't open (.*) for temp writing");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Can't open %1 for temp writing").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_CANNOT_MAKE_TMP);
		}

		rx.setPattern("Can't open (.*) for modifying");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Can't modify file %1").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_CANNOT_MODIFY_FILE);
		}

		rx.setPattern("Error analyzing (.*)\\. This mp3 has some very corrupt data\\.");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Error analyzing %1.").arg(fileName) +
					 " " +
					 tr("This mp3 has some very corrupt data.");
			popupMsg = logMsg;
			throw(ERRTYPE_CORRUPT_MP3);
		}

		rx.setPattern("(.*) is an MPEG Layer I file, not a layer III file");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("%1 is an MPEG Layer I file, not a layer III file").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_FILEFORMAT_NOTSUPPORTED);
		}

		rx.setPattern("(.*) is an MPEG Layer II file, not a layer III file");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("%1 is an MPEG Layer II file, not a layer III file").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_FILEFORMAT_NOTSUPPORTED);
		}

		rx.setPattern("(.*) is free format (not currently supported)");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("%1 is free format (not currently supported)").arg(fileName);
			popupMsg = logMsg;
			throw(ERRTYPE_FILEFORMAT_NOTSUPPORTED);
		}

		/*
		// only for AAC
		if (input == "failed to modify gain") {
			throw(ERRTYPE_FILEFORMAT_NOTSUPPORTED);
		}

		// only for AAC
		rx.setPattern("(.*) is not a valid mp4/m4a file\\.");
		if (rx.indexIn(input) != -1) {
			//QString fileName = rx.cap(1);
			throw(ERRTYPE_FILEFORMAT_NOTSUPPORTED);
		}
		*/

		//QRegExp("Problem re-naming (.*) to (.*)\nThe mp3 was correctly modified, but you will need to re-name (.*) to (.*) yourself\.");
		rx.setPattern("Problem re-naming (.*) to (.*)");
		if (rx.indexIn(input) != -1) {
			QString outFileName = rx.cap(1);
			QString fileName = rx.cap(2);
			logMsg = tr("Problem re-naming %1 to %2.").arg(outFileName).arg(fileName) +
					 " " +
					 tr("The mp3 was correctly modified, but you will need to re-name it yourself.");
			popupMsg = logMsg;
			throw(ERRTYPE_RENAME_TMP);
		}
		rx.setPattern("The mp3 was correctly modified, but you will need to re-name (.*) to (.*) yourself\\.");
		if (rx.indexIn(input) != -1) {
			//QString outFileName = rx.cap(1);
			//QString fileName = rx.cap(2);
			throw(ERRTYPE_SUPPRESSED);
		}

		//QRegExp("Not enough temp space on disk to modify (.*)\nEither free some space, or do not use \"temp file\" option");
		rx.setPattern("Not enough temp space on disk to modify (.*)");
		if (rx.indexIn(input) != -1) {
			QString fileName = rx.cap(1);
			logMsg = tr("Not enough temp space on disk to modify %1.").arg(fileName) +
					 " " +
					 tr("Either clear space on disk, or go to \"Options->Advanced...\" and check the \"Do not use Temp files\" box.");
			popupMsg = logMsg;
			throw(ERRTYPE_NOT_ENOUGH_TEMP_SPACE);
		}
		if (input == "Either free some space, or do not use \"temp file\" option") {
			throw(ERRTYPE_SUPPRESSED);
		}
	}
	catch (ErrType e){
		errType = e;

		if (errType!=ERRTYPE_NONE && errType!=ERRTYPE_SUPPRESSED){
			if (!logMsg.isEmpty()){
				if (logDockWidget->isVisible() && actionLogError->isChecked()){
					// popupMsg omitted, error is only displayed in opened log panel
					popupMsg = QString();
				}
				else{
					if (!actionLogOpenAutomaticallyPanel->isChecked()){
						bool isConfirmSuppressed = (!isOpenLogPanelQuestionSuppressed.isNull() && isOpenLogPanelQuestionSuppressed.toBool())
												   || (settings->contains("openLogPanelForErrorAnswer"));
						int r = QMessageBox::NoButton;
						if (isConfirmSuppressed){
							if (settings->contains("openLogPanelForErrorAnswer")){
								r = settings->value("openLogPanelForErrorAnswer").toBool() ? QMessageBox::Yes : QMessageBox::No;
							}
						}
						else{
							r = MyMessageBox::question(this, appTitle+" - "+tr("Open Log Panel?"),
										tr("Would you like to display the errors in an opening log panel instead of seeing pop-up messages?"),
										tr("Don't ask me again"),
										isConfirmSuppressed,
										QMessageBox::Yes | QMessageBox::No,
										QMessageBox::Yes);
							operationTime.restart();
							if (isConfirmSuppressed){
								settings->setValue("openLogPanelForErrorAnswer", r==QMessageBox::Yes);
							}
							isOpenLogPanelQuestionSuppressed = true;
						}
						if (r == QMessageBox::Yes)
							actionLogOpenAutomaticallyPanel->setChecked(true);
					}
					if (actionLogOpenAutomaticallyPanel->isChecked()){
						// open log panel and set error log checbox, popupMsg can be omitted
						if (!logDockWidget->isVisible())
							logDockWidget->setVisible(true);
						if (!actionLogError->isChecked())
							actionLogError->setChecked(true);
						popupMsg = QString();
					}

				}
			}

			if (!logMsg.isEmpty())
				writeLog(logMsg, LOGTYPE_ERROR);

			if (!popupMsg.isEmpty()){
				bool isConfirmSuppressed = !isPopupErrorSuppressed.isNull() && isPopupErrorSuppressed.toBool();
				if (!isConfirmSuppressed) {
					MyMessageBox::critical(this, appTitle+" - "+tr("Error"),
								popupMsg,
								tr("Don't bother me again with error messages"),
								isConfirmSuppressed,
								QMessageBox::Close);
					operationTime.restart();
					if (isConfirmSuppressed){
						isPopupErrorSuppressed = QVariant(true);
					}
				}
			}
		}
	}

	return errType;
}

int MainWindow::runAnalysis(QModelIndexList indices, bool isAlbum, bool isMaxNoclip, bool isOnlyWithStoredTagInfo, double passSlice){
	QProcess process;
	double startProgress = progressBar_Total->doubleValue();

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		if (indices.isEmpty()) throw(0);

		int total_index = 0;

		QStringList argOptions;

		if (isOnlyWithStoredTagInfo){
			// -s c - only check stored tag info (no other processing)
			argOptions << "-s" << "c";
		}
		if (!isAlbum){
			// -e - make analysis without album stuff
			argOptions << "-e";
		}
		argOptions << getArgumentsByOptions();

		QMultiHash<QString, QModelIndex> indexByPath;

		if (isAlbum){
			// in album mode a process can start only the files belong to their parent path
			// irreversed order is used in iterator trying to keep later the original order
			//foreach(QModelIndex index, indices) {
			QListIterator<QModelIndex> indicesIterator(indices);
			indicesIterator.toBack();
			while (indicesIterator.hasPrevious()){
				QModelIndex index = indicesIterator.previous();
				int row = model->itemFromIndex(index)->row();
				QString pathName = getItemText(row, "Path");
				indexByPath.insert(pathName, index);
				//writeLog(fileName, LOGTYPE_TRACE);
			}
		}
		else{
			indexByPath.insert("dummyPath", QModelIndex());
		}

		foreach(QString pathName, indexByPath.uniqueKeys()){
			QModelIndexList indicesByProcess = isAlbum ? indexByPath.values(pathName) : indices;
			QStringList argFiles;

			foreach(QModelIndex index, indicesByProcess) {
				int row = model->itemFromIndex(index)->row();
				QString fileName = getItemText(row, "Path/File");
				//writeLog(fileName, LOGTYPE_TRACE);
				argFiles << fileName;
			}

			QStringList args = QStringList() << argOptions << argFiles;
			process.setProcessChannelMode(QProcess::MergedChannels);
			QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
			writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

			process.start(this->getBackEnd(), args);

			if (!process.waitForStarted()){
				showNoBackEndVersion();
				throw(1);
			}

			/*
			expected analysis output is a tab separated table, last row contains album info in album mode
			>mp3gain -o "D:/Users/Brazso/music/test_mp3/Wes - Midiwa boi.mp3"
				File	MP3 gain	dB gain	Max Amplitude	Max global_gain	Min global_gain
			  3% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			  6% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			  9% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			 13% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			...
			 94% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			 97% 2562925 bytes analyzed\r (only if not analysis tag info is stored)
			D:/Users/Brazso/music/test_mp3/Wes - Midiwa boi.mp3	0	0.500000	18759.831058	188	111
			"Album"	0	0.500000	18759.831058	188	111 (only in album mode)
			 */

			enum resultColumns { File, MP3_gain, dB_gain, Max_Amplitude, Max_global_gain, Min_global_gain, enumMax };
			enum resultColumnsEx { Album_MP3_gain = enumMax, Album_dB_gain, Album_Max_Amplitude, Album_Max_global_gain, Album_Min_global_gain, enumMaxEx };

			for (bool isAfterLast = false, isWaitForReadyRead = false;
				(isWaitForReadyRead = process.waitForReadyRead(-1)) || !isAfterLast;
				isAfterLast = !isWaitForReadyRead ) {

				QByteArray newData = process.readAll();
				QString result = QString::fromLocal8Bit(newData);
				//writelog(QString("waitForReadyRead: %1").arg(result), LOGTYPE_TRACE);

				QTextStream in(&result);
				do {
					Line line;
					line.content = in.readLine();
					if (line.content==QString::null || line.content.isEmpty())
						continue;

					QStringList lines = line.content.split(QChar('\r'), QString::SkipEmptyParts);
					foreach (line.content, lines){
						if (line.content.trimmed().isEmpty()){
							continue;
						}

						writeLog(line.content, LOGTYPE_BACKEND, line.content.endsWith("bytes analyzed") ? 2 : 1);
						bool isNextIndex = false;
						line.errType = hasError(line.content);

						if (line.errType){
							line.type = LINETYPE_ERROR;
							if (line.errType!=ERRTYPE_SUPPRESSED){
								isNextIndex = true; // for the time being all errors increase the iterator
							}
						}
						else if (line.content.endsWith("bytes analyzed")){ // optional
							line.type = LINETYPE_ANALYSIS;
							// single track: " 23% of 2650308 bytes analyzed"
							// more tracks: "[1/2]  7% of 2650308 bytes analyzed"
							int percent = 0;
							int actFileNumber = 1;
							QRegExp rx("(?:^)(?:\\[(\\d+)(?:/)(\\d+)(?:\\]))?(?: *)(\\d+)(?:% of )(\\d+)(?: bytes analyzed$)");
							int pos = rx.indexIn(line.content);
							if (pos > -1) {
								actFileNumber = rx.cap(1).isEmpty() ? 1 : rx.cap(1).toInt();
								//int totalFileNumber = rx.cap(2).isEmpty() ? 1 : rx.cap(2).toInt();
								percent = rx.cap(3).toInt();
								//long fileSize = rx.cap(4).toLong();
							}
							if (percent>100) percent=100; // some bug from the back end
							setProgress(QVariant(percent),
										QVariant(startProgress+passSlice*((total_index+percent/100.0)/indices.size())));
							updateStatusBar(QString("Analyzing %1").arg(argFiles.at(actFileNumber-1)));
						}
						else {
							QStringList tokens = line.content.split(QChar('\t'));

							bool isIgnore = true;
							if (tokens.size()!=(!isOnlyWithStoredTagInfo ? (int)enumMax : (int)enumMaxEx))
								;
							else if (tokens[File]=="File")
								line.type = LINETYPE_FILE_HEADER;
							else
								isIgnore = false;

							if (!isIgnore){
								double mp3Gain = 0, dbGain = 0, maxAmplitude = 0;
								bool isConvertOk = true;
								if (isConvertOk)
									mp3Gain = tokens[MP3_gain].toInt(&isConvertOk);
								if (isConvertOk)
									dbGain = tokens[dB_gain].toDouble(&isConvertOk);
								if (isConvertOk)
									maxAmplitude = tokens[Max_Amplitude].toDouble(&isConvertOk);
								if (!isConvertOk) {
									if (isOnlyWithStoredTagInfo){
										// we may get NA values here in the tokens
										line.type = LINETYPE_ERROR;
										line.errType=ERRTYPE_SUPPRESSED;
									}
									else{
										line.type = LINETYPE_ERROR;
										line.errType=ERRTYPE_CANNOT_FIND_MP3_FRAME;
										QString msg = QString("%1 %2").arg(tr("Error while analyzing in file")).arg(tokens[File]);
										writeLog(msg, LOGTYPE_ERROR);
									}
								}else{
									updateStatusBar(QString("Analyzing %1").arg(tokens[File]));
								}

								if (!isOnlyWithStoredTagInfo && tokens[File]=="\"Album\""){ // contains album info
									line.type = LINETYPE_FILE_ALBUM;
									if (!line.errType)
										updateModelRowsByAnalysisAlbum(isAlbum, indicesByProcess, mp3Gain, dbGain, QVariant(maxAmplitude), /*isLog =*/ true);
								}else{
									if (!line.errType) {
										if (isOnlyWithStoredTagInfo){
											double dbGainDiff = doubleSpinBox_targetNormalValue->value()-defaultNormalTargetValue;
											dbGain += dbGainDiff;
											mp3Gain = (int)round(dbGain/DB);
										}
										updateModelRowByAnalysisTrack(tokens[File], mp3Gain, dbGain, maxAmplitude, isMaxNoclip,
																	  /*isLog =*/ !isOnlyWithStoredTagInfo);
										if (isOnlyWithStoredTagInfo){
											bool hasAlbumInfo = true;
											int albumMp3Gain(tokens[Album_MP3_gain].toInt(&isConvertOk));
											hasAlbumInfo &= isConvertOk;
											double albumDbGain = tokens[Album_dB_gain].toDouble(&isConvertOk);
											hasAlbumInfo &= isConvertOk;
											double albumMaxAmplitude = tokens[Album_Max_Amplitude].toDouble(&isConvertOk);
											hasAlbumInfo &= isConvertOk;
											if (hasAlbumInfo){
												QList<QStandardItem *> found = model->findItems(tokens[File], Qt::MatchExactly, 0);
												if (found.count()>0) {
													QModelIndex index = model->indexFromItem(found.at(0));
													updateModelRowsByAnalysisAlbum(isAlbum, QModelIndexList() << index, albumMp3Gain, albumDbGain, QVariant(albumMaxAmplitude));
												}
											}
										}
									}
									isNextIndex = true;
								}
							}
						}

						if (isNextIndex){
							total_index++;
							setProgress(QVariant(0),
										QVariant(startProgress+passSlice*(((double)total_index)/indices.size())));
						}

						qApp->processEvents(QEventLoop::AllEvents);
						if (isCancelled) throw(-1);
					}
				} while (!in.atEnd());
			}
			//process.close();
		}

		throw(0);
	}
	catch (int e){
		if (process.state()==QProcess::Starting || process.state()==QProcess::Running){
			process.kill();
		}
		setProgress(QVariant(0), QVariant());
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
		return e;
	}
	return 0;
}

// refresh track fields of a tableView row by mp3Gain track change
void MainWindow::updateModelRowByMP3GainTrack(QString fileName, int mp3Gain, bool isAlbumErase){
	double dBGain = mp3Gain*DB;
	double gainValue = isAlbumErase ? dBGain : 0.0;
	QList<QStandardItem *> found = model->findItems(fileName, Qt::MatchExactly, 0);
	if (found.count()==0) return;
	int row = found.at(0)->row();
	QStandardItem *item = 0;

	item = getItem(row, "Volume");
	if (item && !item->data().isNull())
		setItem(row, "Volume", QVariant(item->data().toDouble()+gainValue));

	QVariant newMaxAmplitude = QVariant(); // double
	item = getItem(row, "Max Amplitude");
	if (item && !item->data().isNull()){
		double maxAmplitude = item->data().toDouble();
		newMaxAmplitude = QVariant(maxAmplitude*pow(2.0, round(gainValue/DB)/4.0));
		setItem(row, "Max Amplitude", newMaxAmplitude);
	}

	// depends on "Max Amplitude" field
	if (!newMaxAmplitude.isNull()){
		item = getItem(row, "clipping");
		if (item && !item->data().isNull()){
			bool clipping = newMaxAmplitude.toDouble()>32767.0;
			setItem(row, "clipping", QVariant(clipping));
		}
	}

	QVariant newGainValue = QVariant(); // double
	item = getItem(row, "Track Gain");
	if (item && !item->data().isNull()){
		newGainValue = QVariant(item->data().toDouble()-dBGain);
		setItem(row, "Track Gain", newGainValue);
	}

	item = getItem(row, "dBGain");
	if (item && !item->data().isNull()){
		double newdBGain = item->data().toDouble()-dBGain;
		setItem(row, "dBGain", QVariant(newdBGain));
	}

	// depends on "Max Amplitude" and "Track Gain" fields
	if (!newMaxAmplitude.isNull() && !newGainValue.isNull()){
		item = getItem(row, "clip(Track)");
		if (item && !item->data().isNull()){
			bool clippingTrack = newMaxAmplitude.toDouble()*pow(2.0, round(newGainValue.toDouble()/DB)/4.0)>32767.0;
			setItem(row, "clip(Track)", QVariant(clippingTrack));
		}
	}

	// depends on "Max Amplitude" field
	if (!newMaxAmplitude.isNull()){
		item = getItem(row, "Max Noclip Gain");
		if (item && !item->data().isNull()){
			int mp3GainNoClip = (int)floor((15.0-log10(newMaxAmplitude.toDouble())/log10(2.0))*4.0);
			setItem(row, "Max Noclip Gain", QVariant(mp3GainNoClip*DB));
		}
	}

	if (isAlbumErase){
		// Album fields cannot be recalculated, so they must be deleted
		QStringList columns = QStringList() << "Album Volume" << "Album Max Amplitude"
							  << "Album Gain" << "Album dBGain" << "clip(Album)";
		foreach (QString element, columns){
			item = getItem(row, element);
			if (item){
				item->setData(0); // before item delete to enable automatic model/view refresh
				delete item;
			}
		}
	}

	// finally set line color to red if clipping is set
	QStringList columns = QStringList() << "Path/File" << "Path" << "File";
	foreach (QString element, columns){
		QStandardItem *item = getItem(row, element);
		item->setForeground(QBrush(QColor(0,0,0)));
	}
	item = getItem(row, "clipping");
	QStandardItem *item1 = getItem(row, "clip(Track)");
	if ((item && item->data().toBool()) || (item1 && item1->data().toBool())){
		if (item) item->setForeground(QBrush(QColor(255,0,0)));
		if (item1) item1->setForeground(QBrush(QColor(255,0,0)));
		foreach (QString element, columns){
			QStandardItem *item = getItem(row, element);
			item->setForeground(QBrush(QColor(255,0,0)));
		}
	}

	// log
	if (mp3Gain!=0) {
		QString trace;
		trace += QString("%1\t").arg(getItemText(row, "Path/File"));
		trace += QString("%1").arg(dBGain);
		writeLog(trace, LOGTYPE_CHANGE);
	}
}

// refresh fields of tableView by mp3Gain album change
void MainWindow::updateModelRowsByMP3GainAlbum(QString fileName, int mp3Gain, bool isTrackModifiable){
	QList<QStandardItem *> found = model->findItems(fileName, Qt::MatchExactly, 0);
	if (found.count()==0) return;
	QModelIndex modelIndex = model->item(found[0]->row())->index();
	updateModelRowsByMP3GainAlbum(QModelIndexList() << modelIndex, mp3Gain, isTrackModifiable);
}

// refresh fields of tableView by mp3Gain album change
void MainWindow::updateModelRowsByMP3GainAlbum(QModelIndexList indices, int mp3Gain, bool isTrackModifiable){
	double dBGain = mp3Gain*DB;
	double gainValue = isTrackModifiable ? dBGain : 0.0;
	QStandardItem *item = 0;

	foreach(QModelIndex index, indices) {
		int row = model->itemFromIndex(index)->row();

		if (isTrackModifiable){
			item = getItem(row, "Volume");
			if (item && !item->data().isNull()){
				setItem(row, "Volume", QVariant(item->data().toDouble()+gainValue));
			}

			QVariant newMaxAmplitude = QVariant(); // double
			item = getItem(row, "Max Amplitude");
			if (item && !item->data().isNull()){
				double maxAmplitude = item->data().toDouble();
				newMaxAmplitude = QVariant(maxAmplitude*pow(2.0, round(gainValue/DB)/4.0));
				setItem(row, "Max Amplitude", QVariant(newMaxAmplitude));
			}

			// depends on "Max Amplitude" field
			if (!newMaxAmplitude.isNull()){
				item = getItem(row, "clipping");
				if (item && !item->data().isNull()){
					bool clipping = newMaxAmplitude.toDouble()>32767.0;
					setItem(row, "clipping", QVariant(clipping));
				}
			}

			QVariant newGainValue = QVariant(); // double
			item = getItem(row, "Track Gain");
			if (item && !item->data().isNull()){
				double newGainValue = item->data().toDouble()-gainValue; // or dBGain ?
				setItem(row, "Track Gain", QVariant(newGainValue));
			}

			item = getItem(row, "dBGain");
			if (item && !item->data().isNull()){
				double newdBGain = item->data().toDouble()-gainValue; // or dBGain ?
				setItem(row, "dBGain", QVariant(newdBGain));
			}

			// depends on "Max Amplitude" and "Track Gain" fields
			if (!newMaxAmplitude.isNull() && !newGainValue.isNull()){
				item = getItem(row, "clip(Track)");
				if (item && !item->data().isNull()){
					bool clippingTrack = newMaxAmplitude.toDouble()*pow(2.0, round(newGainValue.toDouble()/DB)/4.0)>32767.0;
					setItem(row, "clip(Track)", QVariant(clippingTrack));
				}
			}

			// depends on "Max Amplitude" field
			if (!newMaxAmplitude.isNull()){
				item = getItem(row, "Max Noclip Gain");
				if (item && !item->data().isNull()){
					int mp3GainNoClip = (int)floor((15.0-log10(newMaxAmplitude.toDouble())/log10(2.0))*4.0);
					setItem(row, "Max Noclip Gain", mp3GainNoClip*DB);
				}
			}
		}

		item = getItem(row, "Album Volume");
		if (item && !item->data().isNull()){
			//QString value = QString("%1").arg(doubleSpinBox_targetNormalValue->value()-tokens[dB_gain].toDouble(), 0, 'f', 1);
			setItem(row, "Album Volume", QVariant(item->data().toDouble()+gainValue));
		}

		item = getItem(row, "Album Max Amplitude");
		if (item && !item->data().isNull()){
			double maxAlbumAmplitude = item->data().toDouble();
			double newMaxAlbumAmplitude = maxAlbumAmplitude*pow(2.0, round(gainValue/DB)/4.0);
			setItem(row, "Album Max Amplitude", QVariant(newMaxAlbumAmplitude));
		}

		QVariant newGainValue = QVariant(); // double
		item = getItem(row, "Album Gain");
		if (item && !item->data().isNull()){
			newGainValue = QVariant(item->data().toDouble()-dBGain);
			setItem(row, "Album Gain", QVariant(newGainValue));
		}

		item = getItem(row, "Album dBGain");
		if (item && !item->data().isNull()){
			double newAlbumDBGain = item->data().toDouble()-dBGain;
			setItem(row, "Album dBGain", QVariant(newAlbumDBGain));
		}

		// depends on "Max Amplitude" and "Track Gain" fields
		if (!newGainValue.isNull()){
			item = getItem(row, "Max Amplitude");
			if (item && !item->data().isNull()){
				double maxAmplitude = item->data().toDouble();
				double newMaxAmplitude = maxAmplitude*pow(2.0, round(gainValue/DB)/4.0);
				bool clippingTrack = newMaxAmplitude*pow(2.0, round(newGainValue.toDouble()/DB)/4.0)>32767.0;
				setItem(row, "clip(Album)", QVariant(clippingTrack));
			}
		}

		// finally set line color to red if clipping is set
		QStringList columns = QStringList() << "Path/File" << "Path" << "File";
		foreach (QString element, columns){
			QStandardItem *item = getItem(row, element);
			item->setForeground(QBrush(QColor(0,0,0)));
		}
		item = getItem(row, "clipping");
		QStandardItem *item1 = getItem(row, "clip(Track)");
		QStandardItem *item2 = getItem(row, "clip(Album)");
		if ((item && item->data().toBool()) || (item1 && item1->data().toBool()) || (item2 && item2->data().toBool())){
			if (item) item->setForeground(QBrush(QColor(255,0,0)));
			if (item1) item1->setForeground(QBrush(QColor(255,0,0)));
			if (item2) item2->setForeground(QBrush(QColor(255,0,0)));
			foreach (QString element, columns){
				QStandardItem *item = getItem(row, element);
				item->setForeground(QBrush(QColor(255,0,0)));
			}
		}

		// log
		if (mp3Gain!=0) {
			QString trace;
			trace += QString("%1\t").arg(getItemText(row, "Path/File"));
			trace += QString("%1").arg(dBGain);
			writeLog(trace, LOGTYPE_CHANGE);
		}
	}
}

void MainWindow::runGain(QModelIndexList indices, bool isAlbum, double passSlice){
	QProcess process;
	double startProgress = progressBar_Total->doubleValue();

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		if (indices.isEmpty()) throw(0);

		int total_index = 0;

		QStringList argOptions;

		if (isAlbum){
			argOptions << "-a"; // apply Album gain automatically
		}else{
			argOptions << "-r"; // apply Track gain automatically (all files set to equal loudness)
		}
		argOptions << getArgumentsByOptions();

		QMultiHash<QString, QModelIndex> indexByPath;

		if (isAlbum){
			// in album mode a process can start only the files belong to their parent path
			// irreversed order is used in iterator trying to keep later the original order
			//foreach(QModelIndex index, indices) {
			QListIterator<QModelIndex> indicesIterator(indices);
			indicesIterator.toBack();
			while (indicesIterator.hasPrevious()){
				QModelIndex index = indicesIterator.previous();
				int row = model->itemFromIndex(index)->row();
				QString pathName = getItemText(row, "Path");
				indexByPath.insert(pathName, index);
				//writeLog(fileName, LOGTYPE_TRACE);
			}
		}
		else{
			indexByPath.insert("dummyPath", QModelIndex());
		}

		foreach(QString pathName, indexByPath.uniqueKeys()){
			QModelIndexList indicesByProcess = isAlbum ? indexByPath.values(pathName) : indices;
			QStringList argFiles;

			foreach(QModelIndex index, indicesByProcess) {
				int row = model->itemFromIndex(index)->row();
				QString fileName = getItemText(row, "Path/File");
				//writeLog(fileName, LOGTYPE_TRACE);
				argFiles << fileName;
			}

			QStringList args = QStringList() << argOptions << argFiles;
			process.setProcessChannelMode(QProcess::MergedChannels);
			QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
			writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

			process.start(this->getBackEnd(), args);

			if (!process.waitForStarted()){
				showNoBackEndVersion();
				throw(1);
			}

			/*
			new file at track mode:

			line==...bytes analyzed && (prevLine!=...bytes analyzed && prevLine!='File...')
			line==filename && (prevLine!=...bytes analyzed && prevLine!='File...')

			new file at album mode

			line==...bytes analyzed && (prevLine!=...bytes analyzed && prevLine!='File...')
			line==filename && (prevLine!=...bytes analyzed && prevLine!='File...')

			line='Applying mp3 gain change...' && prevLine==...bytes written
			*/

			/*
			track mode: expected undo output is a tab separated table, integer value is the mp3 gain (* 1.5 = real db gain)
			>mp3gain -r -o "2Pac - California Love.mp3" "2Pac - Changes.mp3"
			File    MP3 gain        dB gain Max Amplitude   Max global_gain Min global_gain
			[1/2]  1% of 6848512 bytes analyzed\r (analysis is optional, if no tag is found)
			[1/2]  3% of 6848512 bytes analyzed\r
			...
			[1/2] 98% of 6848512 bytes analyzed\r
			/home/brazso/work/audio/2Pac - California Love.mp3	-7	-10.330000	44325.970901	189	126
			Applying mp3 gain change of -7 to /home/brazso/work/audio/2Pac - California Love.mp3...
			  1% of 6848512 bytes written\r (writing is optional, if not 0 gain change is found)
			  3% of 6848512 bytes written\r
			...
			 98% of 6848512 bytes written\r
			[2/2]  1% of 6465618 bytes analyzed\r
			...
			[2/2] 98% of 6465618 bytes analyzed
			/home/brazso/work/audio/2Pac - Changes.mp3	0	0.290000	16758.632575	182	99
			No changes to /home/brazso/work/audio/2Pac - Changes.mp3 are necessary
			...but tag needs update: Writing tag information for /home/brazso/work/audio/2Pac - Changes.mp3
			*/

			/*
			album mode: expected undo output is a tab separated table, integer value is the mp3 gain (* 1.5 = real db gain)
			>mp3gain -a -o "2Pac - California Love.mp3" "2Pac - Changes.mp3"
			File    MP3 gain        dB gain Max Amplitude   Max global_gain Min global_gain
			[1/2]  1% of 6848834 bytes analyzed\r (analysis is optional, if no tag is found!)
			[1/2]  3% of 6848834 bytes analyzed\r
			...
			[1/2] 98% of 6848834 bytes analyzed\r
			/home/brazso/work/audio/2Pac - California Love.mp3	-7	-10.330000	44325.970901	189	126
			[2/2]  1% of 6465618 bytes analyzed
			...
			[2/2] 98% of 6465618 bytes analyzed
			/home/brazso/work/audio/2Pac - Changes.mp3	-5	-7.230000	39858.970192	187	104
			"Album"	-6	-9.200000	44325.961728	189	104
			( "-c" argument makes this warning and confirmation question out )
			WARNING: /home/brazso/work/audio/2Pac - California Love.mp3 may clip with mp3 gain change -6
			Make change? [y/n]
			Applying mp3 gain change of -6 to /home/brazso/work/audio/2Pac - California Love.mp3...
			  1% of 6848834 bytes written (writing is optional, if not 0 gain change is found)
			...
			 98% of 6848834 bytes written
			Applying mp3 gain change of -6 to /home/brazso/work/audio/2Pac - Changes.mp3...
			  1% of 6465618 bytes written (writing is optional, if not 0 gain change is found)
			...
			 98% of 6465618 bytes written
			*/
			enum resultColumns { File, MP3_gain, dB_gain, Max_Amplitude, Max_global_gain, Min_global_gain, enumMax };
			QList<int> passes = QList<int>() << 90 << 10; // analysis, gain (in track mode)
			bool hasAnalysis = false;
			QStringList tokens;
			Line prevLine;

			for (bool isAfterLast = false, isWaitForReadyRead = false;
				(isWaitForReadyRead = process.waitForReadyRead(-1)) || !isAfterLast;
				isAfterLast = !isWaitForReadyRead ) {

				QByteArray newData = process.readAll();
				QString result = QString::fromLocal8Bit(newData);
				//writeLog(QString("<result>%1</result>").arg(result), LOGTYPE_TRACE);

				QTextStream in(&result);
				do {
					Line line;
					line.content = in.readLine();
					if (line.content==QString::null || line.content.isEmpty())
						continue;

					QStringList lines = line.content.split(QChar('\r'), QString::SkipEmptyParts);
					foreach (line.content, lines){
						if (line.content.trimmed().isEmpty()){
							continue;
						}

						writeLog(line.content, LOGTYPE_BACKEND, line.content.endsWith("bytes analyzed") || line.content.endsWith("bytes written") ? 2 : 1);
						bool isNextIndex = false;
						line.errType = hasError(line.content);

						if (line.errType){
							line.type = LINETYPE_ERROR;
							if (line.errType!=ERRTYPE_SUPPRESSED){
								hasAnalysis = false;
								isNextIndex = true; // for the time being all errors increase the iterator
							}
						}
						else if (line.content.endsWith("bytes analyzed")){ // optional
							line.type = LINETYPE_ANALYSIS;
							// single track: "  5% of 2650308 bytes analyzed"
							// more tracks: "[1/2] 13% of 2650308 bytes analyzed"
							if (prevLine.type!=LINETYPE_ANALYSIS && prevLine.type!=LINETYPE_FILE_HEADER){
								//isNextIndex = true;
								total_index++;
							}
							int percent = 0;
							int actFileNumber = 1;
							QRegExp rx("(?:^)(?:\\[(\\d+)(?:/)(\\d+)(?:\\]))?(?: *)(\\d+)(?:% of )(\\d+)(?: bytes analyzed$)");
							int pos = rx.indexIn(line.content);
							if (pos > -1) {
								actFileNumber = rx.cap(1).isEmpty() ? 1 : rx.cap(1).toInt();
								//int totalFileNumber = rx.cap(2).isEmpty() ? 1 : rx.cap(2).toInt();
								percent = rx.cap(3).toInt();
								//long fileSize = rx.cap(4).toLong();
							}
							if (percent>100) percent=100; // some bug from the back end
							if (!isAlbum)
								percent = (int)round(percent*passes[0]/100.0);
							setProgress(QVariant(percent),
										QVariant(startProgress+passSlice*(((total_index+percent/100.0))/(indices.size()*(isAlbum ? 2 : 1)))));
							updateStatusBar(QString("Analyzing %1").arg(argFiles.at(actFileNumber-1)));
						}
						else if (line.content.endsWith("bytes written")){
							line.type = LINETYPE_WRITTEN;
							// " 43% of 2650308 bytes written"
							int percent = 0;
							QRegExp rx("(?:^ *)(\\d+)(?:% of )(\\d+)(?: bytes written$)");
							int pos = rx.indexIn(line.content);
							if (pos > -1) {
								percent = rx.cap(1).toInt();
								//long fileSize = rx.cap(2).toLong();
							}
							if (percent>100) percent=100; // some bug from the back end
							if (!isAlbum && hasAnalysis)
								percent = (int)(passes[0]+round(percent*passes[1]/100.0));
							setProgress(QVariant(percent),
										QVariant(startProgress+passSlice*(((total_index+percent/100.0))/(indices.size()*(isAlbum ? 2 : 1)))));
							double gainValue = tokens[MP3_gain].toInt()*DB; // dBGain
							updateStatusBar(QString("Applying gain of %1 dB to %2").arg(gainValue, 0, 'f', 1).arg(tokens[File]));
						}
						// Applying mp3 gain change of -5 to /home/brazso/work/audio/2Pac - Changes.mp3...
						// No changes to /home/brazso/work/audio/Alizee - Lolita.mp3 are necessary
						else if ( line.content.startsWith("Applying mp3 gain change of ") ||
								  line.content.startsWith("No changes to ") ){
							line.type = LINETYPE_APPLY_GAIN;
							if (!isAlbum){
								bool isConvertOk;
								double gainValue = tokens[MP3_gain].toInt(&isConvertOk)*DB; // dBGain
								if (isConvertOk){
									QString msg;
									if (gainValue==0)
										msg = tr("No changes to %1").arg(tokens[File]);
									else
										msg = tr("Applying gain of %1 dB to %2").arg(gainValue, 0, 'f', 1).arg(tokens[File]);
									updateStatusBar(msg);
								}
								updateModelRowByAnalysisTrack(tokens[File], tokens[MP3_gain].toInt(), tokens[dB_gain].toDouble(), tokens[Max_Amplitude].toDouble(), /*maxNoclipGain=*/ false, /*isLog=*/ true);
								updateModelRowByMP3GainTrack(tokens[File], tokens[MP3_gain].toInt());
							} else {
								QString fileName;
								QRegExp rx("(?:^Applying mp3 gain change of )(-?\\d+)(?: to )(.*)(?:\\.\\.\\.$)");
								int pos = rx.indexIn(line.content);
								if (pos > -1) {
									QStringList list = rx.capturedTexts();
									writeLog(list.join(" "), LOGTYPE_TRACE);
									if (list.size()==3){
										fileName = rx.cap(2);
										int mp3Gain = rx.cap(1).toInt();
										double gainValue = mp3Gain*DB; // dBGain
										tr("Applying gain of %1 dB to %2").arg(gainValue, 0, 'f', 1).arg(fileName);
									}
								}else{
									rx.setPattern("(?:^No changes to )(.*)(?: are necessary$)");
									pos = rx.indexIn(line.content);
									if (pos > -1) {
										QStringList list = rx.capturedTexts();
										writeLog(list.join(" "), LOGTYPE_TRACE);
										if (list.size()==2){
											fileName = rx.cap(1);
											updateStatusBar(tr("No changes to %1").arg(fileName));
										}
									}
								}
								updateModelRowsByAnalysisAlbum(isAlbum, fileName, tokens[MP3_gain].toInt(), tokens[dB_gain].toDouble(), QVariant(tokens[Max_Amplitude].toDouble()), /*isLog=*/ true);
								updateModelRowsByMP3GainAlbum(fileName, tokens[MP3_gain].toInt());
							}
							//if (prevLine.type!=LINETYPE_FILE_ALBUM){
							//	isNextIndex = true;
							//}
							if (prevLine.type==LINETYPE_WRITTEN || prevLine.type==LINETYPE_APPLY_GAIN || prevLine.type==LINETYPE_FILE_ALBUM){
								isNextIndex = true;
							}
						}
						else {
							QStringList tmpTokens = line.content.split(QChar('\t'));
							if (tmpTokens.size()==enumMax){
								if (tmpTokens[File]=="File"){
									line.type = LINETYPE_FILE_HEADER;
								}
								else{
									line.type = LINETYPE_FILE_CONTENT;
									tokens = tmpTokens;
									if (!isAlbum){
										//updateModelRowByAnalysisTrack(tokens[File], tokens[MP3_gain].toInt(), tokens[dB_gain].toDouble(), tokens[Max_Amplitude].toDouble());
										//updateModelRowByMP3GainTrack(tokens[File], tokens[MP3_gain].toInt());
										hasAnalysis = prevLine.type==LINETYPE_ANALYSIS;
										//isNextIndex = !hasAnalysis && prevLine.type!=LINETYPE_FILE_HEADER;
									}else{
										if (tokens[File]=="\"Album\""){
											line.type = LINETYPE_FILE_ALBUM;
											//updateModelRowsByAnalysisAlbum(isAlbum, indicesByeess, tokens[MP3_gain].toInt(), tokens[dB_gain].toDouble(), QVariant(tokens[Max_Amplitude].toDouble()));
											//updateModelRowsByMP3GainAlbum(indicesByProcess, tokens[MP3_gain].toInt());
										}else{
											bool isConvertOk;
											double gainValue = tokens[MP3_gain].toInt(&isConvertOk)*DB; // dBGain
											if (isConvertOk){
												QString msg;
												if (gainValue==0)
													msg = tr("No changes to %1").arg(tokens[File]);
												else
													msg = tr("Applying gain of %1 dB to %2").arg(gainValue, 0, 'f', 1).arg(tokens[File]);
												updateStatusBar(msg);
											}
											updateModelRowByAnalysisTrack(tokens[File], tokens[MP3_gain].toInt(), tokens[dB_gain].toDouble(), tokens[Max_Amplitude].toDouble(), /*maxNoclipGain=*/ false,/*isLog=*/ true);
											//isNextIndex = true;
										}
									}
									if (line.type == LINETYPE_FILE_CONTENT){
										if (prevLine.type!=LINETYPE_ANALYSIS && prevLine.type!=LINETYPE_FILE_HEADER){
											isNextIndex = true;
										}
									}
								}
							}
						}
						if (isNextIndex){
							total_index++;
							setProgress(QVariant(0),
										QVariant(startProgress+passSlice*(((double)total_index)/(indices.size()*(isAlbum ? 2 : 1)))));
						}

						qApp->processEvents(QEventLoop::AllEvents);
						if (isCancelled) throw(-1);
						prevLine = line;
					} // foreach
				} while (!in.atEnd());
			}
		}

		throw(0);
	}
	catch (int e){
		if (process.state()==QProcess::Starting || process.state()==QProcess::Running){
			process.kill();
		}
		setProgress(QVariant(0), QVariant());
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

void MainWindow::runConstantGain(QModelIndexList indices, int mp3Gain, bool isLeft, bool isRight, double passSlice){
	QProcess process;
	double startProgress = progressBar_Total->doubleValue();

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		if (indices.isEmpty() || mp3Gain==0) throw(0);

		QStringList args;
		if (isLeft && isRight){
			args << "-g" << QString("%1").arg(mp3Gain); // apply gain i to mp3 without doing any analysis
		}else if(isLeft && !isRight){
			args << "-l" << "0" << QString("%1").arg(mp3Gain); // apply gain i to channel 0 (left channel) of mp3 without doing any analysis (ONLY works for STEREO mp3s, not Joint Stereo mp3s)
		}else if(!isLeft && isRight){
			args << "-l" << "1" << QString("%1").arg(mp3Gain); // apply gain i to channel 1 (right channel) of mp3 without doing any analysis (ONLY works for STEREO mp3s, not Joint Stereo mp3s)
		}else{
			throw(1);
		}
		args << getArgumentsByOptions();

		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			QStandardItem *item = getItem(row, "Path/File");
			QString fileName = item->text();
			args << fileName;
		}

		process.setProcessChannelMode(QProcess::MergedChannels);
		QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
		writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		process.start(this->getBackEnd(), args);

		if (!process.waitForStarted()){
			showNoBackEndVersion();
			throw(1);
		}

		/* 2 channels mode, everything goes to SE
		D:\Users\Brazso\music\test_mp3>mp3gain -o -g 1 "Arash - Boro Boro.mp3" "Austin Powers - Theme Song.mp3"
		File    MP3 gain        dB gain Max Amplitude   Max global_gain Min global_gain
		Applying gain change of 1 to Arash - Boro Boro.mp3...
		  1% of 10532812 bytes written
		  3% of 10532812 bytes written
		...
		 99% of 10532812 bytes written
		done
		Applying gain change of 1 to Austin Powers - Theme Song.mp3...
		  2% of 6503689 bytes written
		  5% of 6503689 bytes written
		...
		 97% of 6503689 bytes written
		done
		*/

		/* 1 channel mode
		D:\Users\Brazso\music\test_mp3>mp3gain -o -l 0 1 "Arash - Boro Boro.mp3" "Austin Powers - Theme Song.mp3"
		File    MP3 gain        dB gain Max Amplitude   Max global_gain Min global_gain
		SE:Applying gain change of 1 to CHANNEL 0 of Arash - Boro Boro.mp3...
		SO:Arash - Boro Boro.mp3: Can't adjust single channel for mono or joint stereo
		SE:Applying gain change of 1 to CHANNEL 0 of Austin Powers - Theme Song.mp3...
		SO:Austin Powers - Theme Song.mp3: Can't adjust single channel for mono or joint stereo
		*/

		enum resultColumns { File, MP3_gain, dB_gain, Max_Amplitude, Max_global_gain, Min_global_gain, enumMax };
		int total_index = 0;

		for (bool isAfterLast = false, isWaitForReadyRead = false;
			(isWaitForReadyRead = process.waitForReadyRead(-1)) || !isAfterLast;
			isAfterLast = !isWaitForReadyRead ) {

			QByteArray newData = process.readAll();
			QString result = QString::fromLocal8Bit(newData);

			QTextStream in(&result);
			do {
				Line line;
				line.content = in.readLine();
				if (line.content==QString::null || line.content.isEmpty())
					continue;

				QStringList lines = line.content.split(QChar('\r'), QString::SkipEmptyParts);
				foreach (line.content, lines){
					if (line.content.trimmed().isEmpty()){
						continue;
					}

					writeLog(line.content, LOGTYPE_BACKEND, line.content.endsWith("bytes analyzed") ? 2 : 1);
					bool isNextIndex = false;
					line.errType = hasError(line.content);

					if (line.errType){
						line.type = LINETYPE_ERROR;
						if (line.errType!=ERRTYPE_SUPPRESSED){
							isNextIndex = true; // for the time being all errors increase the iterator
						}
					}
					else if (line.content.endsWith("bytes written")){
						line.type = LINETYPE_WRITTEN;
						// " 43% of 2650308 bytes written"
						int percent = 0;
						QRegExp rx("(?:^ *)(\\d+)(?:% of )(\\d+)(?: bytes written$)");
						int pos = rx.indexIn(line.content);
						if (pos > -1) {
							percent = rx.cap(1).toInt();
							//long fileSize = rx.cap(2).toLong();
						}
						if (percent>100) percent=100; // some bug from the back end
						setProgress(QVariant(percent),
									QVariant(startProgress+passSlice*(((total_index+percent/100.0))/indices.size())));
					}
					else if (line.content=="done"){
						isNextIndex = true;
					}
					else{
						QStringList tokens = line.content.split(QChar('\t'));

						if (tokens.size()==enumMax && tokens[File]=="File"){
							line.type = LINETYPE_FILE_HEADER;
						}
						else{
							QRegExp rx("(?:Applying gain change of )(-?\\d+)(?: to )(.*)(?:\\.\\.\\.)");
							if (!(isLeft && isRight)){
								rx=QRegExp("(?:Applying gain change of )(-?\\d+)(?: to CHANNEL \\d of )(.*)(?:\\.\\.\\.)");
							}
							int pos = rx.indexIn(line.content);
							if (pos > -1) {
								line.type = LINETYPE_APPLY_GAIN;
								QStringList list = rx.capturedTexts();
								if (list.size()==3){
									QString fileName = rx.cap(2);
									int mp3Gain = rx.cap(1).toInt();
									updateModelRowByMP3GainTrack(fileName, mp3Gain);
								}
							}else{
								// error message?
							}
						}
					}
					if (isNextIndex){
						total_index++;
						setProgress(QVariant(),
									QVariant(startProgress+passSlice*(((double)total_index)/indices.size())));
					}

					qApp->processEvents(QEventLoop::AllEvents);
					if (isCancelled) throw(-1);
				}
			} while (!in.atEnd());
		}

		throw(0);
	}
	catch (int e){
		if (process.state()==QProcess::Starting || process.state()==QProcess::Running){
			process.kill();
		}
		setProgress(QVariant(0), QVariant());
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// -----
// slots
// -----

void MainWindow::showContextMenuForWidget(const QPoint &pos){
	QStandardItem *item = getItem(tableView->currentIndex().row(), "Path/File");
	QString fileName = item->text();
	writeLog(fileName, LOGTYPE_TRACE);

    // create context menu for tableView
	QMenu contextMenu(this);
	bool checkBox_Maximizing = settings->value("checkBox_Maximizing", false).toBool();

	// unfortunately original actions cannot be used, because we must know
	// that the sender object is from contextMenu or MainWindow
	QVector<QAction*> actions;
	actions << actionTrack_Analysis;
	actions << actionAlbum_Analysis;
	if (checkBox_Maximizing){
		actions << actionMax_No_clip_Analysis;
	}
	actions << 0;
	actions << actionClear_Analysis;
	actions << 0;
	actions << actionTrack_Gain;
	actions << actionAlbum_Gain;
	actions << actionConstant_Gain;
	if (checkBox_Maximizing){
		actions << actionMax_No_clip_Gain_for_Each_file;
		actions << actionMax_No_clip_Gain_for_Album;
	}
	actions << 0;
	actions << actionUndo_Gain_changes;
	actions << 0;
	actions << actionRemove_Tags_from_files;

	if (!actionPlay_mp3_file){
		actionPlay_mp3_file = new QAction(this);
		//actionPlay_mp3_file->setObjectName(QString::fromUtf8("actionPlay_mp3_file"));
		connect(actionPlay_mp3_file, SIGNAL(triggered()), this, SLOT(playMP3File()));
	}
	bool isToBeStopped = false;
	if (mediaObject && mediaObject->state()==Phonon::PlayingState){
		QString playedFileName = mediaObject->currentSource().fileName();
		if (fileName==playedFileName){
			isToBeStopped = true;
		}

	}
	actionPlay_mp3_file->setText(!isToBeStopped ? tr("&Play mp3 file") : tr("Stop &playing mp3 file"));

	actions << 0;
	actions << actionPlay_mp3_file;

	foreach (QAction* action, actions){
		if (action){
			contextMenu.addAction(action);
			action->setProperty("calledFromContextMenu", true);
		}else{
			contextMenu.addSeparator();
		}
	}

	contextMenu.exec(QCursor::pos());

	foreach (QAction* action, actions){
		if (action){
			action->setProperty("calledFromContextMenu", false);
		}
	}
}

void MainWindow::loadDonationUrlStarted(){
	if (donationView)
		disableGUI();
}

void MainWindow::loadDonationUrlProgress(int progress){
	setProgress(QVariant(), QVariant(progress));
}

void MainWindow::loadDonationUrlFinished(bool isLoadFinished){
	if (donationView && !isCancelled) {
		if (!isLoadFinished){
			writeLog(QString("Donation URL cannot be opened by QWebView: %1").arg(donationUrl), LOGTYPE_TRACE);
			isLoadFinished = QDesktopServices::openUrl(donationUrl);
			if (!isLoadFinished){
				writeLog(QString("Donation URL cannot be opened by QDesktopServices: %1").arg(donationUrl), LOGTYPE_TRACE);
				QMessageBox::critical(this, appTitle,
						tr("Could not open donation URL"));
				operationTime.restart();
				delete donationView;
			}
		}else{
			donationView->show();
		}
	}
	enableGUI();
}

// process called once in this order, the first existing one gains:
// !backEndFileName: this->backEndFileName, this->backEndFixed
//  backEndFileName: backEndFileName, this->backEndFixed
QString MainWindow::findBackEndVersionByProcess(const QString & backEndFileName){
	QString result("");

	try{
		QProcess process;
		QString backEnd = !backEndFileName.isNull() ? (!backEndFileName.isEmpty() ? backEndFileName : this->getBackEndFixed()) : this->getBackEnd();
		QStringList args = QStringList() << "-v";

		process.setProcessChannelMode(QProcess::MergedChannels);
		QString trace = QString("%1 %2").arg(backEnd).arg(args.join(" "));
		writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		process.start(backEnd, args);

		if (!process.waitForStarted()){
			throw(1);
		}

		if (!process.waitForFinished(1000)) throw(1);

		QByteArray newData = process.readAll();
		// expected result #1: mp3gain version 1.4.6<line end>
		// expected result #2: C:\Program Files\MP3Gain\mp3gain.exe version 1.4.6<line end>
		result = QString::fromLocal8Bit(newData);
		writeLog(result, LOGTYPE_BACKEND);
		result = result.simplified(); // removing <line end>
		int index = result.indexOf(QRegExp(" version "));
		if (index == -1)
			throw(1);
		result = result.mid(index);
		result = result.section(' ', 2, 2); // getting pure version

		throw(0);
	}
	catch (int e){
	}

	return result;
}

void MainWindow::showNoBackEndVersion(bool isStartBackEndDialog){
	QString msg = tr("MP3Gain back end cannot be found.\n"
					 "Please make it available!");
	if (isStartBackEndDialog){
		if (!backEndVersion.isEmpty()){
			msg = tr("Required MP3Gain back end version is %1 or later, but only %2 is found.\n"
					 "Please make a newer version available!").arg(this->requiredBackEndVersion).arg(backEndVersion);
		}
		QMessageBox::warning(this, appTitle, msg);
		operationTime.restart();
		on_actionBack_end_triggered();
	}else{
		QMessageBox::critical(this, appTitle, msg);
		operationTime.restart();
	}
}

void MainWindow::updateModelRowsByNewTargetNormalValue(double newTargetNormalValue){
	static double oldTargetNormalValue = settings->value("doubleSpinBox_targetNormalValue", defaultNormalTargetValue).toDouble();
	if (oldTargetNormalValue!=newTargetNormalValue){
		int mp3Gain, oldMp3Gain, newMp3Gain;
		double albumVolume = 0.0;
		QModelIndexList indices;

		// Tracks
		for (int row=0; row<model->rowCount(); row++){
			QStandardItem* item = getItem(row, "Volume");
			if (item){
				double volume = item->data().toDouble();
				oldMp3Gain = (int)round((oldTargetNormalValue - volume)/DB);
				newMp3Gain = (int)round((newTargetNormalValue - volume)/DB);
				mp3Gain = oldMp3Gain-newMp3Gain;
				if (mp3Gain!=0){
					updateModelRowByMP3GainTrack(getItemText(row, "Path/File"), mp3Gain, false);
				}
				item = getItem(row, "Album Volume");
				if (item){
					albumVolume = item->data().toDouble();
					indices << getItemIndex(row, "Path/File");
				}
			}
		}

		// Album
		oldMp3Gain = (int)round((oldTargetNormalValue - albumVolume)/DB);
		newMp3Gain = (int)round((newTargetNormalValue - albumVolume)/DB);
		mp3Gain = oldMp3Gain-newMp3Gain;
		updateModelRowsByMP3GainAlbum(indices, mp3Gain, false);

		oldTargetNormalValue = newTargetNormalValue;
	}
}

// menu: File/Load_Analysis_results
void MainWindow::on_actionLoad_Analysis_results_triggered(){
	QString fileMainName = QFileDialog::getOpenFileName(this, tr("Open analysis file"), lastAddedFolder, tr("Comma-separated files (*.m3g *.csv);;XML files (*.xml)"));
	if (fileMainName.isEmpty()) return;

	QFile fileMain;
	QTextStream inFileMain;
	QDomDocument docMain;
	QDomElement rootElement;
	//QDomNode fileNode; // child of rootNode
	QDomElement fileElement;

	try{
		QFileInfo fi(fileMainName);
		bool isXML = fi.suffix().toLower()=="xml";

		fileMain.setFileName(fileMainName);
		if (!isXML){
			if (!fileMain.open(QIODevice::ReadOnly)) {
	//    		cerr << "Cannot open file for reading: "
	//        	 	<< qPrintable(file.errorString()) << endl;
				throw(-1);
			}
			inFileMain.setDevice(&fileMain);
		}
		else
		{
			QString errorMsg;
			int errorLine;
			int errorColumn;
			if (!docMain.setContent(&fileMain, true, &errorMsg, &errorLine,
									&errorColumn)) {
				QMessageBox::warning(0, tr("DOM Parser"),
									 tr("Parse error at line %1, column %2:\n%3")
									 .arg(errorLine)
									 .arg(errorColumn)
									 .arg(errorMsg));
				operationTime.restart();
				throw -1;
			}
			rootElement = docMain.documentElement();
			if (rootElement.tagName() != "root")
				throw -1;
			fileElement = rootElement.firstChildElement("file");
		}

		bool isYesToAll_FileExist = false;
		bool isNoToAll_FileExist = false;
		bool isYesToAll_FileLastModified = false;
		bool isNoToAll_FileLastModified = false;
		bool isYesToAll_FileSize = false;
		bool isNoToAll_FileSize = false;
		//in.setCodec("UTF-8");

		while ( (!isXML && !inFileMain.atEnd()) || (isXML && !fileElement.isNull()) ) {
			bool isAnalysisIgnored = false;
			bool isConvertOk;

			// fields of file from analysis file
			QString fieldPath;
			QString fieldFile;
			QDateTime fieldFileLastModified;
			qint64 fieldFileSize = 0;
			QVariant fieldMaxAmplitude(QVariant::Double);
			QVariant fieldDBGain(QVariant::Double);
			QVariant fieldAlbumDBGain(QVariant::Double);

			if (!isXML) {
				QString line = inFileMain.readLine();
				QStringList fields = line.split(',');
				if (fields.size()!=7)
					throw(-1);

				fieldPath = fields[0];
				fieldPath.remove(QRegExp("^\"")).remove(QRegExp("\"$"));

				fieldFile = fields[1];
				fieldFile.remove(QRegExp("^\"")).remove(QRegExp("\"$"));

				QString value = fields[2];
				value.remove(QRegExp("^#")).remove(QRegExp("#$"));
				fieldFileLastModified = QDateTime::fromString(value, "yyyy-MM-dd hh:mm:ss");

				fieldFileSize = fields[3].toLongLong(&isConvertOk);

				double doubleValue = fields[4].toDouble(&isConvertOk);
				if (isConvertOk)
					fieldMaxAmplitude = QVariant(doubleValue);

				doubleValue = fields[5].toDouble(&isConvertOk);
				if (isConvertOk)
					fieldDBGain = QVariant(doubleValue);

				doubleValue = fields[6].toDouble(&isConvertOk);
				if (isConvertOk)
					fieldAlbumDBGain = QVariant(doubleValue);
			}
			else /* isXML */ {
				QDomElement fieldElement = fileElement.firstChildElement();
				while (!fieldElement.isNull()) {
					if (fieldElement.tagName()=="path") {
						fieldPath = fieldElement.text();
					}
					else if (fieldElement.tagName()=="file") {
						fieldFile = fieldElement.text();
					}
					else if (fieldElement.tagName()=="file_last_modified") {
						fieldFileLastModified = QDateTime::fromString(fieldElement.text(), "yyyy-MM-dd hh:mm:ss");
					}
					else if (fieldElement.tagName()=="file_size") {
						fieldFileSize = fieldElement.text().toLongLong(&isConvertOk);
					}
					else if (fieldElement.tagName()=="max_amplitude") {
						double doubleValue = fieldElement.text().toDouble(&isConvertOk);
						if (isConvertOk)
							fieldMaxAmplitude = QVariant(doubleValue);
					}
					else if (fieldElement.tagName()=="db_gain") {
						double doubleValue = fieldElement.text().toDouble(&isConvertOk);
						if (isConvertOk)
							fieldDBGain = QVariant(doubleValue);
					}
					else if (fieldElement.tagName()=="album_db_gain") {
						double doubleValue = fieldElement.text().toDouble(&isConvertOk);
						if (isConvertOk)
							fieldAlbumDBGain = QVariant(doubleValue);
					}
					fieldElement = fieldElement.nextSiblingElement();
				}
				fileElement = fileElement.nextSiblingElement("file");
			}

			QString fileName = fieldPath + "/" + fieldFile;
			fi = QFileInfo(fileName);
			if (!fi.exists()){
				continue;
			}

			if (!isAnalysisIgnored && fi.lastModified()!=fieldFileLastModified){
				if (isNoToAll_FileLastModified){
					//continue;
					isAnalysisIgnored = true;
				}
				else if (!isYesToAll_FileLastModified){
					//QString("%1").arg(mp3Gain)
					QString text = tr("File may have been modified after analysis was saved:\n"
								   "%1\n"
								   "Load saved analysis results anyhow?").arg(fileName);
					int button = QMessageBox::warning(this, appTitle, text,
										 QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel,
										 QMessageBox::Yes);
					operationTime.restart();

					if (button==QMessageBox::YesToAll){
						isYesToAll_FileLastModified = true;
					}
					else if (button==QMessageBox::No){
						//continue;
						isAnalysisIgnored = true;
					}
					else if (button==QMessageBox::NoToAll){
						isAnalysisIgnored = true;
						isNoToAll_FileLastModified = true;
					}
					else if (button==QMessageBox::Cancel){
						break;
					}
				}
			}

			if (!isAnalysisIgnored && fi.size()!=fieldFileSize){
				if (isNoToAll_FileSize){
					//continue;
					isAnalysisIgnored = true;
				}
				else if (!isYesToAll_FileSize){
					QString text = tr("File size changed after analysis was saved:\n"
							   "%1\n"
							   "Load saved analysis results anyhow?").arg(fileName);
					int button = QMessageBox::warning(this, appTitle, text,
										 QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel,
										 QMessageBox::Yes);
					operationTime.restart();

					if (button==QMessageBox::YesToAll){
						isYesToAll_FileSize = true;
					}
					else if (button==QMessageBox::No){
						//continue;
						isAnalysisIgnored = true;
					}
					else if (button==QMessageBox::NoToAll){
						isAnalysisIgnored = true;
						isNoToAll_FileSize = true;
					}
					else if (button==QMessageBox::Cancel){
						break;
					}
				}
			}

			QList<QStandardItem *> found = model->findItems(fileName, Qt::MatchExactly, 0);
			bool isNew = found.count()==0;
			int row = -1;
			if (!isNew){
				if (isNoToAll_FileExist){
					//continue;
					isAnalysisIgnored = true;
				}
				if (false && !isYesToAll_FileExist){ // TODO: perhaps it should be activated in general settings
					QString text = tr("File already exists in list:\n"
								   "%1\n"
								   "Load saved analysis results anyhow?").arg(fileName);
					int button = QMessageBox::warning(this, appTitle, text,
										 QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel,
										 QMessageBox::Yes);
					operationTime.restart();

					if (button==QMessageBox::YesToAll){
						isYesToAll_FileExist = true;
					}
					else if (button==QMessageBox::No){
						//continue;
						isAnalysisIgnored = true;
					}
					else if (button==QMessageBox::NoToAll){
						isAnalysisIgnored = true;
						isNoToAll_FileExist = true;
					}
					else if (button==QMessageBox::Cancel){
						break;
					}
				}
				row = found.at(0)->row();
			}else{
				// new row/file
				QStandardItem *item0 = new QStandardItem(fileName);
				QStandardItem *item1 = new QStandardItem(fieldPath);
				QStandardItem *item2 = new QStandardItem(fieldFile);
				model->appendRow(QList<QStandardItem *>() << item0 << item1 << item2);
				row = model->indexFromItem(item0).row();
			}

			if (isAnalysisIgnored)
				continue;

			// load analyis result for file
			if (!fieldDBGain.isNull()) {
				int mp3Gain = (int)round(fieldDBGain.toDouble()/DB);
				updateModelRowByAnalysisTrack(fileName, mp3Gain, fieldDBGain.toDouble(), fieldMaxAmplitude.toDouble());
				if (!fieldAlbumDBGain.isNull()) {
					int mp3AlbumGain = (int)round(fieldAlbumDBGain.toDouble()/DB);
					updateModelRowsByAnalysisAlbum(true, QModelIndexList() << model->item(row)->index(),
												   mp3AlbumGain, fieldAlbumDBGain.toDouble());
				}
			}

		} // while

		throw(0);
	}
	catch (int e){
		if (fileMain.isOpen())
			fileMain.close();
	}
}

// menu: File/Save_Analysis_results
void MainWindow::on_actionSave_Analysis_results_triggered(){
	QString fileMainName = QFileDialog::getSaveFileName(this, tr("Save analysis file"), lastAddedFolder, tr("Comma-separated files (*.m3g *.csv);;XML files (*.xml)"));
	if (fileMainName.isEmpty()) return;

	QFile fileMain;
	try{
		QFileInfo fi(fileMainName);
		if (fi.suffix().isEmpty()) fileMainName += ".m3g";
		bool isXML = fi.suffix().toLower()=="xml";

		fileMain.setFileName(fileMainName);
		if (!fileMain.open(QIODevice::WriteOnly)) {
//    		cerr << "Cannot open file for writing: "
//        	 	<< qPrintable(file.errorString()) << endl;
    		throw(-1);
		}

		QTextStream out(&fileMain);
		//out.setCodec("UTF-8");

		QModelIndexList indices = getModelIndices();

		if (isXML)
			out << "<root>" << endl;
		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			QStandardItem *item = 0;

			if (isXML)
				out << "\t<file>" << endl;

			item = getItem(row, "Path");
			if (item){
				if (isXML)
					out << "\t\t<path>" << QString(item->text().toHtmlEscaped()) << "</path>" << endl;
				else
					out << "\"" << item->text() << "\"" << ",";
			}
			item = getItem(row, "File");
			if (item){
				if (isXML)
					out << "\t\t<file>" << QString(item->text().toHtmlEscaped()) << "</file>" << endl;
				else
					out << "\"" << item->text() << "\"" << ",";
			}
			item = getItem(row, "Path/File");
			if (item) {
				QFileInfo fi(item->text());
				QDateTime dateTime = fi.lastModified();
				if (isXML)
					out << "\t\t<file_last_modified>" << QString(dateTime.toString("yyyy-MM-dd hh:mm:ss").toHtmlEscaped()) << "</file_last_modified>" << endl;
				else
					out << "#" << dateTime.toString("yyyy-MM-dd hh:mm:ss") << "#" << ",";
				if (isXML)
					out << "\t\t<file_size>" << fi.size() << "</file_size>" << endl;
				else
					out << fi.size() << ",";
			}
			item = getItem(row, "Max Amplitude");
			if (item){
				double maxAmplitude = item->data().toDouble();
				QString maxAmplitudeStr =  QString("%1").arg(maxAmplitude, 0, 'f', 3);
				if (isXML)
					out << "\t\t<max_amplitude>" << QString(maxAmplitudeStr.toHtmlEscaped()) << "</max_amplitude>" << endl;
				else
					out << maxAmplitudeStr << ",";
			}
			item = getItem(row, "dBGain");
			if (item){
				double dBGain = item->data().toDouble();
				QString dBGainStr =  QString("%1").arg(dBGain, 0, 'f', 3);
				if (isXML)
					out << "\t\t<db_gain>" << QString(dBGainStr.toHtmlEscaped()) << "</db_gain>" << endl;
				else
					out << dBGainStr << ",";
			}
			item = getItem(row, "Album dBGain");
			if (item){
				double AlbumdBGain = item->data().toDouble();
				QString AlbumdBGainStr =  QString("%1").arg(AlbumdBGain, 0, 'f', 3);
				if (isXML)
					out << "\t\t<album_db_gain>" << QString(AlbumdBGainStr.toHtmlEscaped()) << "</album_db_gain>" << endl;
				else
					out << AlbumdBGainStr;
			}
			if (isXML)
				out << "\t</file>" << endl;
			else
				out << endl;
		}
		if (isXML)
			out << "</root>" << endl;

		throw(0);
	}
	catch (int e){
		if (fileMain.isOpen())
			fileMain.close();
	}
}

// menu: File/Add_files
void MainWindow::on_actionAdd_Files_triggered(){
	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add MP3 file(s)"), lastAddedFolder, tr("MP3 files (*.mp3)"));
	if (fileNames.isEmpty()) return;

	try {
		disableGUI();

		// calculate passes
		QList<int> passes = operationMap["add_file"];
		if (!actionDon_t_check_adding_files->isChecked()){
			passes = operationMap["add_file, analysis"];
		}

		lastAddedIndices = QModelIndexList();

		// pass #1
		int pass = 1;
		for (int i = 0; i < fileNames.size(); ++i) {
			// storing opened folder for later usage
			QFileInfo fi(fileNames.at(i));
			if (i==0) lastAddedFolder = fi.absolutePath();

			// check that actual filename is already stored
			QList<QStandardItem *> found = model->findItems(fileNames.at(i), Qt::MatchExactly, 0);
			if (found.count()>0) continue;
			QStandardItem *item0 = new QStandardItem(fi.absoluteFilePath());
			QStandardItem *item1 = new QStandardItem(fi.absolutePath());
			QStandardItem *item2 = new QStandardItem(fi.fileName());
			model->appendRow(QList<QStandardItem *>() << item0 << item1 << item2);
			QModelIndex index = model->indexFromItem(item0);
			if (index.isValid()){
				lastAddedIndices.append(index);
			}

			setProgress(QVariant(),
						QVariant((int)(passes.at(pass-1)*(((double)(i+1))/fileNames.size()))));
			qApp->processEvents(QEventLoop::AllEvents);
			if (isCancelled) throw(-1);

		}

		if (!actionDon_t_check_adding_files->isChecked() && !lastAddedIndices.isEmpty()){
			pass ++; // pass #2
			if (runAnalysis(lastAddedIndices, true, false, true, (double)passes.at(pass-1))==-1)
				throw(-1);
		}

		throw(0);
	}
	catch (int e){
		refreshMenu();
		enableGUI();
	}
}

// enlist model with mp3 files of folder (it may be recursive)
// menu: File/Add_folder
void MainWindow::on_actionAdd_Folder_triggered(){
	QString dir = QFileDialog::getExistingDirectory(this, tr("Add Directory"), lastAddedFolder, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()) return;

	try {
		disableGUI();

		lastAddedIndices = QModelIndexList();

		// calculate passes
		QList<int> passes = operationMap["add_folder"];
		if (!actionDon_t_check_adding_files->isChecked()){
			passes = operationMap["add_folder, analysis"];
		}
		int pass = 1;

		//tableView->hide();
		if (getMP3FilesByFolder(dir, 0, (double)passes.at(pass-1))==-1)
			throw(-1);
		//tableView->show();
		lastAddedFolder = dir;

		if (!actionDon_t_check_adding_files->isChecked() && !lastAddedIndices.isEmpty()){
			pass ++; // pass #2
			if (runAnalysis(lastAddedIndices, true, false, true, (double)passes.at(pass-1))==-1)
				throw(-1);
		}
		throw(0);
	}
	catch (int e){
		refreshMenu();
		enableGUI();	}
}

// menu: File/Select_all_files
void MainWindow::on_actionSelect_All_Files_triggered(){
	tableView->selectAll();
}

// menu: File/Select_no_files
void MainWindow::on_actionSelect_No_Files_triggered(){
	tableView->clearSelection();
}

// menu: File/Invert_selection
void MainWindow::on_actionInvert_selection_triggered(){
	QItemSelectionModel *selectionModel = tableView->selectionModel();
	QModelIndex topLeft = model->index(0, 0);
	QModelIndex bottomRight = model->index(model->rowCount()-1, model->columnCount()-1);
	QItemSelection selection(topLeft, bottomRight);
	selectionModel->select(selection, QItemSelectionModel::Toggle);
}

// menu: File/Clear_selected_files
void MainWindow::on_actionClear_Selected_Files_triggered(){
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice = 100;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QItemSelectionModel *selectionModel = tableView->selectionModel();
		QModelIndexList indices = selectionModel->selectedRows();
		if (indices.isEmpty()) throw(0);

		// index.row() is not updated if an element is deleted from the model
		// therefore the rows must be deleted from the higher to the lower positions
		QList<int> list;
		foreach(QModelIndex index, indices) {
			list.append(index.row());
		}
		qSort(list.begin(), list.end(), qGreater<int>()); // inverse sort

		for (int i = 0; i < list.size(); ++i){
			model->removeRow(list[i]);
			setProgress(QVariant(), QVariant(i+1));
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
			if (isCancelled) throw(-1);
		}

		refreshMenu();

		throw(0);
	}
	catch (int e){
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: File/Clear_all_files
void MainWindow::on_actionClear_All_files_triggered(){
	model->removeRows(0, model->rowCount());
	refreshMenu();
}

// menu: Analysis/Track_analysis
void MainWindow::on_actionTrack_Analysis_triggered(){
	updateStatusBar(tr("Track analysis started..."));
	runAnalysis(getModelIndices(), false, false);
	updateStatusBar("");
}

// menu: Analysis/Album_analysis
void MainWindow::on_actionAlbum_Analysis_triggered(){
	runAnalysis(getModelIndices(), true, false);
}

// menu: Analysis/Max No-clip Analysis
void MainWindow::on_actionMax_No_clip_Analysis_triggered(){
	runAnalysis(getModelIndices(), false, true);
}

// menu: Analysis/Clear_analysis
void MainWindow::on_actionClear_Analysis_triggered(){
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice = 100.0;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		bool isConfirmSuppressed = settings->value("clearAnalysis_ConfirmSuppressed", false).toBool();
		if (!isConfirmSuppressed){
			int r = MyMessageBox::question(this, appTitle+" - "+tr("Clear Analysis?"),
						tr("This will clear all analysis results.\n"
						   "Are you sure?"),
						tr("Don't ask me again"),
						isConfirmSuppressed,
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::No);
			operationTime.restart();
			if (r == QMessageBox::No) {
				throw(0);
			}
			if (isConfirmSuppressed){
				settings->setValue("clearAnalysis_ConfirmSuppressed", true);
			}
		}

		for (int i = 0; i<indices.size(); i++) {
			int row = model->itemFromIndex(indices.at(i))->row();

			// columns to be get rid of their colors
			QStringList columns = QStringList() << "Path/File" << "Path" << "File";
			foreach (QString element, columns){
				QStandardItem *item = getItem(row, element);
				item->setForeground(QBrush(QColor(0,0,0)));
			}
			// columns to be deleted
			columns = QStringList() << "Volume" << "Max Amplitude" << "clipping"
					  << "Track Gain" << "dBGain" << "clip(Track)"
					  << "Max Noclip Gain"<< "Album Volume" << "Album Max Amplitude"
					  << "Album Gain" << "Album dBGain" << "clip(Album)";
			foreach (QString element, columns){
				QStandardItem *item = getItem(row, element);
				if (item) {
					item->setData(0); // before item delete to enable automatic model/view refresh
					delete item;
				}
			}

			setProgress(QVariant(),
						QVariant(startProgress+passSlice*(((double)(i+1))/indices.size())));
			qApp->processEvents(QEventLoop::AllEvents);
			if (isCancelled) throw(-1);
		}
		throw(0);
	}
	catch (int e){
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: Modify_gain/Track_gain
void MainWindow::on_actionTrack_Gain_triggered(){
	runGain(getModelIndices(), false);
}

// menu: Modify_gain/Album_gain
void MainWindow::on_actionAlbum_Gain_triggered(){
	runGain(getModelIndices(), true);
}

// menu: Modify_gain/Constant_gain
void MainWindow::on_actionConstant_Gain_triggered(){
	try {
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		ConstantGainChangeDialog dialog(this);
		if (!dialog.exec()) throw(0);

		int mp3Gain = dialog.horizontalSlider_constGainChange->value();
		if (mp3Gain!=0){
			bool channelLeft = !dialog.groupBox_onlyOneChannel->isChecked() || dialog.radioButton_channelLeft->isChecked();
			bool channelRight = !dialog.groupBox_onlyOneChannel->isChecked() || dialog.radioButton_channelRight->isChecked();
			runConstantGain(indices, mp3Gain, channelLeft, channelRight);
		}
		throw(0);
	}
	catch (int e){
	}
}

// menu: Modify_gain/Max No-clip Gain for Each file
// Description: This function is rather slow, because mp3gain backend
//              must be called on each mp3 file one by one.
void MainWindow::on_actionMax_No_clip_Gain_for_Each_file_triggered(){
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice =  100.0;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		// calculate passes
		QList<int> passes = operationMap["max_no_clip_gain_for_each_file"];

		// pass #1
		runAnalysis(indices, false, true, false, passes.at(0));

		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			QStandardItem *item = getItem(row, "Max Noclip Gain");
			int maxNoclipGain = (int)round(item->data().toDouble()/DB);
			if (maxNoclipGain!=0){
				// pass #2(,3,...)
				runConstantGain(QModelIndexList() << index, maxNoclipGain, true, true, passes.at(1)/indices.size());
			}
		}
		throw(0);
	}
	catch (int e){
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: Modify_gain/Max No-clip Gain for Album
void MainWindow::on_actionMax_No_clip_Gain_for_Album_triggered(){
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice = 100.0;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		// calculate passes
		QList<int> passes = operationMap["max_no_clip_gain_for_album"];

		// pass #1
		runAnalysis(indices, false, true, false, passes.at(0));

		int minMaxNoclipGain = INT_MAX;
		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			QStandardItem *item = getItem(row, "Max Noclip Gain");
			int maxNoclipGain = (int)round(item->data().toDouble()/DB);
			if (maxNoclipGain<minMaxNoclipGain){
				minMaxNoclipGain = maxNoclipGain;
			}
		}

		if (minMaxNoclipGain!=INT_MAX && minMaxNoclipGain!=0.0){
			// pass #2
			runConstantGain(indices, minMaxNoclipGain, true, true, (double)passes.at(1));
		}

		throw(0);
	}
	catch (int e){
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: Modify_gain/Undo_gain_changes
void MainWindow::on_actionUndo_Gain_changes_triggered(){
	QProcess process;
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice = 100.0;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		QStringList args;
		args << "-u"; // undo changes made by mp3gain (based on stored tag info)
		args << getArgumentsByOptions();

		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			QStandardItem *item = getItem(row, "Path/File");
			QString fileName = item->text();
			args << fileName;
		}

		process.setProcessChannelMode(QProcess::MergedChannels);
		QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
		writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		process.start(this->getBackEnd(), args);

		if (!process.waitForStarted()){
			showNoBackEndVersion();
			throw(1);
		}

		// File    left global_gain change right global_gain change
		/*
		expected undo output is a tab separated table, integer value is the mp3 gain (* 1.5 = real db gain)
		D:\Users\Brazso\music\test_mp3>mp3gain -u -o "Arash - Boro Boro.mp3" "Austin Powers - Theme Song.mp3"
		File    left global_gain change right global_gain change
		Arash - Boro Boro.mp3   7       7
		1% of 10533134 bytes written (optional, if not 0 gain is found)
		3% of 10533134 bytes written
		...
		99% of 10533134 bytes written
		Austin Powers - Theme Song.mp3  0       0
		2% of 6504011 bytes written (optional, if not 0 gain is found)
		5% of 6504011 bytes written
		...
		97% of 6504011 bytes written
		*/
		enum resultColumns { File, left_global_gain_change, right_global_gain_change, enumMax };
		int total_index = 0;
		Line prevLine;

		for (bool isAfterLast = false, isWaitForReadyRead = false;
			(isWaitForReadyRead = process.waitForReadyRead(-1)) || !isAfterLast;
			isAfterLast = !isWaitForReadyRead ) {

			QByteArray newData = process.readAllStandardOutput();
			QString result = QString::fromLocal8Bit(newData);

			QTextStream in(&result);
			do {
				Line line;
				line.content = in.readLine();
				if (line.content==QString::null || line.content.isEmpty())
					continue;

				QStringList lines = line.content.split(QChar('\r'), QString::SkipEmptyParts);
				foreach (line.content, lines){
					if (line.content.trimmed().isEmpty()){
						continue;
					}

					writeLog(line.content, LOGTYPE_BACKEND, line.content.endsWith("bytes written") ? 2 : 1);
					bool isNextIndex = false;
					line.errType = hasError(line.content);

					if (line.errType){
						line.type = LINETYPE_ERROR;
						if (line.errType!=ERRTYPE_SUPPRESSED){
							isNextIndex = true; // for the time being all errors increase the iterator
						}
					}
					else if (line.content.endsWith("bytes written")){
						line.type = LINETYPE_WRITTEN;
						// " 12% of 7260121 bytes written"
						int percent = 0;
						QRegExp rx("(?:^ *)(\\d+)(?:% of )(\\d+)(?: bytes written$)");
						int pos = rx.indexIn(line.content);
						if (pos > -1) {
							percent = rx.cap(1).toInt();
							//long fileSize = rx.cap(2).toLong();
						}
						if (percent>100) percent=100; // some bug from the back end
						setProgress(QVariant(percent),
									QVariant(startProgress+passSlice*(((total_index+percent/100.0))/indices.size())));
					}
					else{
						QStringList tokens = line.content.split(QChar('\t'));

						if (tokens.size()==enumMax){
							if (tokens[File]=="File"){
								line.type = LINETYPE_FILE_HEADER;
							}
							else{
								line.type = LINETYPE_FILE_CONTENT;
								//writeLog(tokens[File], LOGTYPE_TRACE);
								updateModelRowByMP3GainTrack(tokens[File], (tokens[left_global_gain_change].toInt()+tokens[right_global_gain_change].toInt())/2);
								if (prevLine.type!=LINETYPE_FILE_HEADER && !prevLine.errType)
									isNextIndex = true;
							}
						}
					}

					if (isNextIndex){
						total_index++;
						setProgress(QVariant(),
									QVariant(startProgress+passSlice*(((double)total_index)/indices.size())));
					}

					qApp->processEvents(QEventLoop::AllEvents);
					if (isCancelled) throw(-1);
					prevLine = line;
				} // foreach
			} while (!in.atEnd());
		}
		throw(0);
	}
	catch (int e){
		if (process.state()==QProcess::Starting || process.state()==QProcess::Running){
			process.kill();
		}
		setProgress(QVariant(0), QVariant());
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: Options/Always_on_Top
void MainWindow::on_actionAlways_on_Top_toggled(bool checked){
	Qt::WindowFlags flags = this->windowFlags();
	if (checked){
		flags |= Qt::WindowStaysOnTopHint;
	}
	else{
		flags &= ~Qt::WindowStaysOnTopHint;
	}
	this->setWindowFlags(flags);
	this->show();
}

// menu: Options/Tags/Remove_Tags_from_files
void MainWindow::on_actionRemove_Tags_from_files_triggered(){
	QProcess process;
	double startProgress = progressBar_Total->doubleValue();
	const double passSlice = 100.0;

	try {
		if (startProgress==0.0 && passSlice==100.0) disableGUI();
		QModelIndexList indices = getModelIndices();
		if (indices.isEmpty()) throw(0);

		QStringList args;
		args << "-s" << "d"; // delete stored tag info (no other processing)
		args << getArgumentsByOptions();

		foreach(QModelIndex index, indices) {
			int row = model->itemFromIndex(index)->row();
			//QStandardItem *item = model->itemFromIndex(index);
			QStandardItem *item = getItem(row, "Path/File");
			QString fileName = item->text();
			//writelog(fileName, LOGTYPE_TRACE);
			//fileName = QDir::toNativeSeparators(fileName);
			//writelog(fileName, LOGTYPE_TRACE);
			args << fileName;
		}

		process.setProcessChannelMode(QProcess::MergedChannels);
		QString trace = QString("%1 %2").arg(this->getBackEnd()).arg(args.join(" "));
		writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		process.start(this->getBackEnd(), args);

		if (!process.waitForStarted()){
			showNoBackEndVersion();
			throw(1);
		}

		/*
		expected output is a tab separated table, where only the first column (File) is filled
		File    MP3 gain        dB gain Max Amplitude   Max global_gain Min global_gain
		2Pac - California Love.mp3	NA	NA	NA	NA	NA
		2Pac - Changes.mp3	NA	NA	NA	NA	NA
		*/

		enum resultColumns { File, MP3_gain, dB_gain, Max_Amplitude, Max_global_gain, Min_global_gain, enumMax };

		int total_index = 0;

		for (bool isAfterLast = false, isWaitForReadyRead = false;
			(isWaitForReadyRead = process.waitForReadyRead(-1)) || !isAfterLast;
			isAfterLast = !isWaitForReadyRead ) {

			QByteArray newData = process.readAll();
			QString result = QString::fromLocal8Bit(newData);
			//writeLog(QString("waitForReadyRead: %1").arg(result), LOGTYPE_TRACE);

			QTextStream in(&result);
			do {
				Line line;
				line.content = in.readLine();
				if (line.content==QString::null || line.content.isEmpty())
					continue;

				QStringList lines = line.content.split(QChar('\r'), QString::SkipEmptyParts);
				foreach (line.content, lines){
					if (line.content.trimmed().isEmpty()){
						continue;
					}

					writeLog(line.content, LOGTYPE_BACKEND);
					bool isNextIndex = false;
					line.errType = hasError(line.content);

					if (line.errType){
						line.type = LINETYPE_ERROR;
						if (line.errType!=ERRTYPE_SUPPRESSED){
							isNextIndex = true; // for the time being all errors increase the iterator
						}
					}
					else{
						QStringList tokens = line.content.split(QChar('\t'));
						if (tokens.size()==enumMax){
							if (tokens[File]=="File")
								line.type = LINETYPE_FILE_HEADER;
							else{
								line.type = LINETYPE_FILE_CONTENT;
								isNextIndex = true;
							}
						}
					}

					if (isNextIndex){
						total_index++;
						setProgress(QVariant(),
									QVariant(startProgress+passSlice*(((double)total_index)/indices.size())));
					}

					qApp->processEvents(QEventLoop::AllEvents);
					if (isCancelled) throw(-1);
				}
			} while (!in.atEnd());
		}

		throw(0);
	}
	catch (int e){
		if (process.state()==QProcess::Starting || process.state()==QProcess::Running){
			process.kill();
		}
		if (startProgress==0.0 && passSlice==100.0){
			enableGUI();
		}
	}
}

// menu: Options/Logs/File...
void MainWindow::on_actionLogFile_triggered(){
	// modal dialog
	LogOptionsDialog dialog(this);
	if (dialog.exec()) {
	}
}

// menu: Options/Logs/Panel
void MainWindow::on_actionLogDock_triggered(){
	refreshGUI();
}

// menu: Options/Toolbar/
void MainWindow::on_actionBig_triggered(){
	actionSmall->setChecked(false);
	actionText_only->setChecked(false);
	actionNone->setChecked(false);
	refreshGUI();
}

void MainWindow::on_actionSmall_triggered(){
	actionBig->setChecked(false);
	actionText_only->setChecked(false);
	actionNone->setChecked(false);
	refreshGUI();
}

void MainWindow::on_actionText_only_triggered(){
	actionBig->setChecked(false);
	actionSmall->setChecked(false);
	actionNone->setChecked(false);
	refreshGUI();
}

void MainWindow::on_actionNone_triggered(){
	actionBig->setChecked(false);
	actionSmall->setChecked(false);
	actionText_only->setChecked(false);
	refreshGUI();
}

// menu: Options/Filename_Display/
void MainWindow::on_actionShow_Path_slash_File_triggered(){
	actionShow_File_only->setChecked(false);
	actionShow_Path_at_File->setChecked(false);
	refreshGUI();
}

void MainWindow::on_actionShow_File_only_triggered(){
	actionShow_Path_slash_File->setChecked(false);
	actionShow_Path_at_File->setChecked(false);
	refreshGUI();
}

void MainWindow::on_actionShow_Path_at_File_triggered(){
	actionShow_Path_slash_File->setChecked(false);
	actionShow_File_only->setChecked(false);
	refreshGUI();
}

// menu: Options/Beep when finished
void MainWindow::on_actionBeep_when_finished_toggled(bool checked){
	if (checked){
		if (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty()){
			QString resourcePath = directoryOf("resources/sounds").absolutePath();
			QString beepFile(resourcePath+"/"+"beep.wav");
			if (QFileInfo(beepFile).exists())
				beepSound = new QSound(beepFile);
			else
				writeLog(tr("Beep %1 file cannot be found").arg(beepFile), LOGTYPE_ERROR);
		}
		else{
			writeLog(tr("QSound is unavailable, QApplication::beep() will be used instead"), LOGTYPE_TRACE);
		}
	}
	else{
		delete beepSound;
	}
}

// menu: Options/Reset "Warning" messages
void MainWindow::on_actionReset_warning_messages_triggered(){
	settings->setValue("clearAnalysis_ConfirmSuppressed", false);
	settings->setValue("clearLogs_ConfirmSuppressed", false);
	settings->setValue("constantGainChangeDialog/groupBox_onlyOneChannel_ConfirmSuppressed", false);
	settings->remove("openLogPanelForErrorAnswer");
}

// menu: Options/Reset Default Column Widths
void MainWindow::on_actionReset_default_column_widths_triggered(){
	//tableView->resizeColumnsToContents();
	// restore default widths of tableView
	QList<QVariant> widths = tableView->property("columnWidths").toList(); // QList<int>
	for (int i=0; i<tableView->model()->columnCount(); i++){
		tableView->setColumnWidth(i, widths.at(i).toInt());
	}
}

// menu: Options/Resize column widths to contents
void MainWindow::on_actionResize_column_widths_by_content_triggered(){
	tableView->resizeColumnsToContents();
}

// menu: Options/Back end...
void MainWindow::on_actionBack_end_triggered(){
	// modal dialog
	BackEndDialog dialog(this);
	if (dialog.exec()) {
	}
}

// menu: Options/Advanced...
void MainWindow::on_actionAdvanced_triggered(){
	// modal dialog
	AdvancedOptionsDialog* dialog = new AdvancedOptionsDialog(this);
	//dialog->groupBox_threadPriority->setVisible(false);
	if (dialog->exec()){
	}
	delete dialog; // call destructor directly to store options before refreshGUI
	refreshGUI();
}

// menu: Help/Contents
void MainWindow::on_actionContents_triggered(){
	QProcess *process = new QProcess;
	QStringList args;
	QString app;
#if defined(Q_OS_WIN)
	app += QLatin1String("assistant.exe");
#elif defined(Q_OS_MAC)
	app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#else //defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
	app += QLatin1String("assistant");
#endif
	QFileInfo fi(app);
	if (!fi.exists()){
		app = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator() + app;
	}
	QString locale = settings->value("locale", QLocale::system().name()).toString();

	fi.setFile(directoryOf("help").absolutePath(), "qmp3gain_"+locale+".qhc");
	if (!fi.exists()){
		if (locale!=defaultLocale){
			writeLog(tr("Help file %1 cannot be found").arg(fi.absoluteFilePath()), LOGTYPE_TRACE);
		}
		fi.setFile(directoryOf("help").absolutePath(), "qmp3gain.qhc");
	}
	if (fi.exists()){
		args << QLatin1String("-collectionFile")
			<< fi.absoluteFilePath().toLatin1()
			<< QLatin1String("-enableRemoteControl");
		writeLog(tr("Help file %1 is used").arg(fi.absoluteFilePath()), LOGTYPE_TRACE);
		QString trace = QString("%1 %2").arg(app).arg(args.join(" "));
		writeLog(trace, LOGTYPE_BACKEND, 1, LOGOPTION_BOLD);

		process->start(app, args);
		if (!process->waitForStarted())
			return;
	}
	else{
		writeLog(tr("Help file %1 cannot be found").arg(fi.absoluteFilePath()), LOGTYPE_ERROR);
		delete process;
	}
}

// menu: Help/Disclaimer
void MainWindow::on_actionDisclaimer_triggered(){
	// modal dialog
	DisclaimerDialog dialog(this);
    if (dialog.exec()) {
    }
}

// menu: Help/About
void MainWindow::on_actionAbout_triggered(){
	// modal dialog
	AboutDialog aboutDialog(this);
	if (aboutDialog.exec()) {
//		QString str = dialog.lineEdit->text().toUpper();
//		spreadsheet->setCurrentCell(str.mid(1).toInt() - 1, str[0].unicode() - 'A');
	}
/* modeless dialog
	if (!aboutDialog){
		aboutDialog = new AboutDialog(this);
		//connect(findDialog, SIGNAL(findNext(const QString &, Qt::CaseSensitivity)), spreadsheet, SLOT(findNext(const QString &, Qt::CaseSensitivity)));
	}
	if (aboutDialog->isHidden()){
		aboutDialog->show();
	}else{
		aboutDialog->raise();
		aboutDialog->activateWindow();
	}
*/
}
