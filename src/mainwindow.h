#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QProcess>
#include <QErrorMessage>
#include <QTranslator>
#include <QDir>
#include <QPointer>
#include <QWebView>
#include <QSound>
#include <QSystemTrayIcon>
#include <QTime>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();
	inline QPointer<QSettings> getSettings() { return settings; }
	inline static const QString & getAppTitle() { return appTitle; }
	inline static const QString & getAppVersion() { return appVersion; }
	inline static const QString & getAppLastCommitId() { return appLastCommitId; }
	inline static const QString & getAppLastCommitDate() { return appLastCommitDate; }
	inline const QString & getBackEnd() { return backEndFileName.isEmpty() ? backEndFixed : backEndFileName; }
	inline static const QString & getBackEndFixed() { return backEndFixed; }
	inline static const QString & getDonationUrl() { return donationUrl; }
	inline static const QString & getRequiredBackEndVersion() { return requiredBackEndVersion; }
	inline QString & getBackEndFileName() { return backEndFileName; }
	inline void setBackEndFileName(const QString & backEndFileName) { this->backEndFileName = backEndFileName; }
	inline QString & getBackEndVersion() { return backEndVersion; }
	inline void setBackEndVersion(const QString & backEndVersion) { this->backEndVersion = backEndVersion; }
	inline bool isBackEndAvailable(const QString & backEndVersion = QString()) { return getVersionNumber(!backEndVersion.isEmpty() ? backEndVersion : this->backEndVersion)>=getVersionNumber(this->requiredBackEndVersion); }
	inline QPointer<QFile> getFileLog() { return fileLog; }
	inline void setFileLog(QPointer<QFile> fileLog) { this->fileLog = fileLog; }
	inline QPointer<QWebView> getDonationView() { return donationView; }
	inline void setDonationView(QPointer<QWebView> donationView) { this->donationView = donationView; }
	inline QStandardItem* getItem(int row, const QString & column) { return model->item(row, modelHeaderList.indexOf(column)); }
	inline QModelIndex getItemIndex(int row, const QString & column) { return model->index(row, modelHeaderList.indexOf(column)); }
	long getVersionNumber(const QString & versionString);
	void showNoBackEndVersion(bool isStartBackEndDialog = false);
	QString findBackEndVersionByProcess(const QString & backEndFileName = QString());
	enum LogType { LOGTYPE_ERROR, LOGTYPE_ANALYSIS, LOGTYPE_CHANGE, LOGTYPE_BACKEND, LOGTYPE_TRACE };
	enum LogOptionFlag { LOGOPTION_NONE = 0x0000, LOGOPTION_BOLD = 0x0001 };
	Q_DECLARE_FLAGS(LogOption, LogOptionFlag)
	enum ErrType { ERRTYPE_NONE = 0,
				   ERRTYPE_ANALYSING_MAX_TIME_REACHED,
				   ERRTYPE_CANCELLED_PROCESSING,
				   ERRTYPE_CANCELLED_PROCESSING_CORRUPT,
				   ERRTYPE_CANNOT_ADJUST_SINGLE_CHANNEL,
				   ERRTYPE_CANNOT_FIND_MP3_FRAME,
				   ERRTYPE_CANNOT_MAKE_TMP,
				   ERRTYPE_CANNOT_MODIFY_FILE,
				   ERRTYPE_CORRUPT_MP3,
				   ERRTYPE_FILEFORMAT_NOTSUPPORTED,
				   ERRTYPE_RENAME_TMP,
				   ERRTYPE_NOT_ENOUGH_TEMP_SPACE,
				   ERRTYPE_SUPPRESSED /* pseudo, usually part of another error */ };
	/*
	enum OpType { OPTYPE_ADD_FILE, OPTYPE_ADD_FOLDER, OPTYPE_ALBUM_ANALYSIS,
				  OPTYPE_ALBUM_GAIN, OPTYPE_ANALYSIS, OPTYPE_CLEAR_ANALYSIS,
				  OPTYPE_CONSTANT_GAIN, OPTYPE_MAX_NO_CLIP_ANALYSIS,
				  OPTYPE_MAX_NO_CLIP_GAIN_FOR_EACH_FILE, OPTYPE_MAX_NO_CLIP_GAIN_FOR_ALBUM,
				  OPTYPE_TRACK_ANALYSIS, OPTYPE_TRACK_GAIN, OPTYPE_UNDO_GAIN_CHANGES };
	*/
	enum LineType { LINETYPE_UNDEFINED = 0,
					LINETYPE_ERROR,
					LINETYPE_ANALYSIS,
					LINETYPE_WRITTEN,
					LINETYPE_APPLY_GAIN,
					LINETYPE_FILE_HEADER,
					LINETYPE_FILE_ALBUM,
					LINETYPE_FILE_CONTENT };
	struct Line
	{
		QString content;
		LineType type;
		ErrType errType;

		Line() : content(QString()), type(LINETYPE_UNDEFINED), errType(ERRTYPE_NONE)
		{
		}
	};

public slots:
	void switchLanguage(QAction *action = 0);

protected:
    void closeEvent(QCloseEvent *event);
	void changeEvent (QEvent *event);
	//void showEvent(QShowEvent *event);
	void createStatusBar();

private:
	QPointer<QTranslator> appTranslator;
	QPointer<QActionGroup> menuLanguageActionGroup;
	QMap<QString, QList<int> > operationMap;
	QVector<QAction*> actions;
	QMap<QAction*, QCheckBox*> logOutputTypes;
	QPointer<QSettings> settings;
	QString lastAddedFolder;
	QModelIndexList lastAddedIndices;
	const char ** modelHeaderLabels;
	QStringList modelHeaderList; // contains column labels (always in english) of model header
	QPointer<QStandardItemModel> model;
	const static QString appTitle;
	const static QString appVersion;
	const static QString appLastCommitId;
	const static QString appLastCommitDate;
	const static QString backEndFixed;
	const static QString donationUrl;
	const static QString requiredBackEndVersion;
	QString backEndFileName;
	QString backEndVersion;
	bool backEndAvailable;
	const static double defaultNormalTargetValue;
	const static QString defaultLocale;
	const static double DB;
	QSize iconDefaultSize;
	bool isCancelled;
	bool enabledGUI;
	QVariant isPopupErrorSuppressed; // bool, exists just inside a function
	QVariant isOpenLogPanelQuestionSuppressed; // bool, exists just inside a function
	QPointer<QFile> fileLog;
	QPointer<QWebView> donationView;
	QPointer<QLabel> messageLabel;
	QPointer<QLabel> modelRowCountLabel;
	QPointer<QSound> beepSound;
	QTime operationTime;
	QByteArray mainGeometry;

	QPointer<QAction> restoreTrayAction;
	QPointer<QAction> quitTrayAction;
	QPointer<QSystemTrayIcon> trayIcon;

	bool okToContinue();
	QDir directoryOf(const QString &subdir);
	void createLanguageMenu();
	void writeSettings();
	void readSettings();
	void setProgress(QVariant progressFile, QVariant progressTotal);
	int getMP3FilesByFolder(const QString & dir, const int level, const double passSlice = 100.0);
	void refreshUi();
	void enableGUI();
	void disableGUI();
	void refreshGUI();
	void refreshMenu();
	QStringList getArgumentsByOptions();
	QString getItemText(int row, const QString & column);
	QVariant getItemValue(int row, const QString & column);
	void setItem(int row, QString column, QVariant value);
	void writeLog(const QString & msg, LogType logType, int level = 1, LogOption logOption = LOGOPTION_NONE);
	void updateModelRowByAnalysisTrack(QString fileName, int mp3Gain, double dBGain, double maxAmplitude, bool maxNoclipGain = false, bool isLog = false);
	void updateModelRowByAnalysisTrack(QModelIndex modelIndex, int mp3Gain, double dBGain, double maxAmplitude, bool maxNoclipGain = false, bool isLog = false);
	void updateModelRowsByAnalysisAlbum(bool isAlbum, QString fileName, int mp3Gain, double dBGain, /*double*/ QVariant maxAmplitude = QVariant(QVariant::Double), bool isLog = false);
	void updateModelRowsByAnalysisAlbum(bool isAlbum, QModelIndexList indexes, int mp3Gain, double dBGain, /*double*/ QVariant maxAmplitude = QVariant(QVariant::Double), bool isLog = false);
	QModelIndexList getModelIndices();
	ErrType hasError(const QString & line);
	int runAnalysis(QModelIndexList indices, bool isAlbum = false, bool isMaxNoclip = false, bool isOnLastAddedFiles = false, double passSlice = 100.0);
	void updateModelRowByMP3GainTrack(QString fileName, int mp3Gain, bool isAlbumErase = true);
	void updateModelRowsByMP3GainAlbum(QString fileName, int mp3Gain, bool isTrackModifiable = true);
	void updateModelRowsByMP3GainAlbum(QModelIndexList indices, int mp3Gain, bool isTrackModifiable = true);
	void runGain(QModelIndexList indices, bool isAlbum = false, double passSlice = 100.0);
	void runConstantGain(QModelIndexList indices, int mp3gain, bool isLeft, bool isRight, double passSlice = 100.0);
	void updateStatusBar(const QString & msg);
	void createTrayIcon();

private slots:
	void showContextMenuForWidget(const QPoint &pos);
	void updateModelRowsByNewTargetNormalValue(double newTargetNormalValue);
	void loadDonationUrlStarted();
	void loadDonationUrlProgress(int progress);
	void loadDonationUrlFinished(bool isLoadFinished);
	void updateStatusBar();
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void trayShowMessage();
	void trayHide();

	void on_cancelButton_clicked();
	void on_clearLogButton_clicked();
	void on_logDockWidget_visibilityChanged(bool visible);

	// menu: File/Load_Analysis_results
	void on_actionLoad_Analysis_results_triggered();

	// menu: File/Save_Analysis_results
	void on_actionSave_Analysis_results_triggered();

	// menu: File/Add_files
	void on_actionAdd_Files_triggered();

	// menu: File/Add_folder
	void on_actionAdd_Folder_triggered();

	// menu: File/Select_all_files
	void on_actionSelect_All_Files_triggered();

	// menu: File/Select_no_files
	void on_actionSelect_No_Files_triggered();

	// menu: File/Invert_selection
	void on_actionInvert_selection_triggered();

	// menu: File/Clear_selected_files
	void on_actionClear_Selected_Files_triggered();

	// menu: File/Clear_all_files
	void on_actionClear_All_files_triggered();

	// menu: Analysis/Track_analysis
	void on_actionTrack_Analysis_triggered();

	// menu: Analysis/Album_analysis
	void on_actionAlbum_Analysis_triggered();

	// menu: Analysis/Max No-clip Analysis
	void on_actionMax_No_clip_Analysis_triggered();

	// menu: Analysis/Clear_analysis
	void on_actionClear_Analysis_triggered();

	// menu: Modify_gain/Track_gain
	void on_actionTrack_Gain_triggered();

	// menu: Modify_gain/Album_gain
	void on_actionAlbum_Gain_triggered();

	// menu: Modify_gain/Constant_gain
	void on_actionConstant_Gain_triggered();

	// menu: Modify_gain/Max No-clip Gain for Each file
	void on_actionMax_No_clip_Gain_for_Each_file_triggered();

	// menu: Modify_gain/Max No-clip Gain for Album
	void on_actionMax_No_clip_Gain_for_Album_triggered();

	// menu: Modify_gain/Undo_gain_changes
	void on_actionUndo_Gain_changes_triggered();

	// menu: Options/Always_on_Top
	void on_actionAlways_on_Top_toggled(bool checked);

	// menu: Options/Tags/Remove_Tags_from_files
	void on_actionRemove_Tags_from_files_triggered();

	// menu: Options/Logs/File...
	void on_actionLogFile_triggered();

	// menu: Options/Logs/Panel
	void on_actionLogDock_triggered();

	// menu: Options/Toolbar/
	void on_actionBig_triggered();
	void on_actionSmall_triggered();
	void on_actionText_only_triggered();
	void on_actionNone_triggered();

	// menu: Options/Filename_Display/
	void on_actionShow_Path_slash_File_triggered();
	void on_actionShow_File_only_triggered();
	void on_actionShow_Path_at_File_triggered();

	// menu: Options/Beep when finished
	void on_actionBeep_when_finished_toggled(bool checked);

	// menu: Options/Reset "Warning" messages
	void on_actionReset_warning_messages_triggered();

	// menu: Options/Reset Default Column Widths
	void on_actionReset_default_column_widths_triggered();

	// menu: Options/Resize column widths by content
	void on_actionResize_column_widths_by_content_triggered();

	// menu: Options/Back end...
	void on_actionBack_end_triggered();

	// menu: Options/Advanced...
	void on_actionAdvanced_triggered();

	// menu: Help/About
	void on_actionContents_triggered();
	void on_actionDisclaimer_triggered();
	void on_actionAbout_triggered();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MainWindow::LogOption)

#endif
