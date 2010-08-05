; qmp3gain.nsi
;
; This script must be compiled with Nullsoft Scriptable Install
; System. See more info here: http://nsis.sourceforge.net
;
; It will install qmp3gain.exe and optionally mp3gain.exe backend
; and/or QT4 dlls.
; Moreover creates a qmp3gain_uninstall.exe for uninstallation.

;--------------------------------

!define NAME "QMP3Gain"
!define FILENAME "qmp3gain"
; qmp3gain.nsh file is created by "make install" and it contains VERSION
!include "qmp3gain.nsh"
#!define VERSION "0.9.0.b7cffcf"

; Compression type
SetCompressor lzma

; The name of the installer
Name "${NAME} ${VERSION}"

; The file to write
OutFile "${FILENAME}-${VERSION}-install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\${NAME}

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "SOFTWARE\Zematix\${NAME}" "Install_Dir"

;--------------------------------

; Pages

Page license
Page components
Page directory
Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; First is default
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\Hungarian.nlf"

; License data
LicenseLangString myLicenseData ${LANG_ENGLISH} "..\..\COPYING"
LicenseLangString myLicenseData ${LANG_HUNGARIAN} "..\..\COPYING"
LicenseData $(myLicenseData)

; Set name using the normal interface (Name command)
LangString Name ${LANG_ENGLISH} "English"
LangString Name ${LANG_HUNGARIAN} "Hungarian"

InstType "Full"
InstType "Minimal"

;--------------------------------

; Support of storing installed filenames
; for easy uninstallation

!define UninstLog "${FILENAME}_uninstall.log"
Var UninstLog
 
; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation of files cannot proceed, delete the files manually!"
LangString UninstLogMissing ${LANG_HUNGARIAN} "${UninstLog} nem található!$\r$\nEltávolítása a fájloknak nem elvégezhetõ, törölje azokat manuálisan!"
 
; AddItem macro
!macro AddItem Path
	FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
	IfFileExists "$OUTDIR\${FileName}" +2
		FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
	File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; CreateShortcut macro
!macro CreateShortcut FilePath FilePointer
	FileWrite $UninstLog "${FilePath}$\r$\n"
	CreateShortcut "${FilePath}" "${FilePointer}"
!macroend
!define CreateShortcut "!insertmacro CreateShortcut"
 
; Copy files macro
!macro CopyFiles SourcePath DestPath
	IfFileExists "${DestPath}" +2
		FileWrite $UninstLog "${DestPath}$\r$\n"
	CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"
 
; Rename macro
!macro Rename SourcePath DestPath
	IfFileExists "${DestPath}" +2
		FileWrite $UninstLog "${DestPath}$\r$\n"
	Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"
 
; CreateDirectory macro
!macro CreateDirectory Path
	CreateDirectory "${Path}"
	FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
	SetOutPath "${Path}"
	FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
; WriteUninstaller macro
!macro WriteUninstaller Path
	WriteUninstaller "${Path}"
	FileWrite $UninstLog "$OUTDIR\${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"
 
Section -openlogfile
	CreateDirectory "$INSTDIR"
	IfFileExists "$INSTDIR\${UninstLog}" +3
		FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
	Goto +4
		SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
		FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
		FileSeek $UninstLog 0 END
SectionEnd

;--------------------------------

; A LangString for the function StartMenuGroupSelect
LangString MUI_STARTMENUPAGE_TEXT_TOP ${LANG_ENGLISH} "Select the Start Menu folder in which you would like to create the program's shortcuts:"
LangString MUI_STARTMENUPAGE_TEXT_TOP ${LANG_HUNGARIAN} "Adja meg a start menü bejegyzés nevét amelybe a program rövidítések kerülnek:"
LangString MUI_STARTMENUPAGE_TEXT_CHECKBOX ${LANG_ENGLISH} "Don't create a start menu folder"
LangString MUI_STARTMENUPAGE_TEXT_CHECKBOX ${LANG_HUNGARIAN} "Ne hozzon létre start menü bejegyzést"

Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /text "$(MUI_STARTMENUPAGE_TEXT_TOP)" /checknoshortcuts "$(MUI_STARTMENUPAGE_TEXT_CHECKBOX)" /autoadd /lastused $R0 "${NAME}"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $R0 "${NAME}" # use default
		Return
	success:
	Pop $R0

	done:
	Pop $R1
FunctionEnd

;--------------------------------

; A LangString for the section name
LangString Sec1Name ${LANG_ENGLISH} "${NAME} (required)"
LangString Sec1Name ${LANG_HUNGARIAN} "${NAME} (kötelezõ)"

; The stuff to install
Section !$(Sec1Name)

	SectionIn RO

	; Set output path to the installation directory.
	${SetOutPath} $INSTDIR
	
	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\Zematix\${NAME} "Install_Dir" "$INSTDIR"
	WriteRegStr HKLM SOFTWARE\Zematix\${NAME} "Version" "${VERSION}"
	WriteRegStr HKLM SOFTWARE\Zematix\${NAME} "Language" $LANGUAGE
	
	; Write the uninstall keys for Windows
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayName" "${NAME} ${VERSION}"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "UninstallString" '"$INSTDIR\${FILENAME}_uninstall.exe"'
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "NoModify" 1
	WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "NoRepair" 1
	${WriteUninstaller} "${FILENAME}_uninstall.exe"

	; Put files there
	${File} "generated\" "${FILENAME}.exe"
	;${File} "generated\" "${FILENAME}_readme.txt"
	${SetOutPath} $INSTDIR\help
	${File} "generated\help\" "qmp3gain.qch"
	${File} "generated\help\" "qmp3gain.qhc"
	${SetOutPath} $INSTDIR\translations
	${File} "generated\translations\" "qmp3gain_hu.qm"

	ClearErrors
	SearchPath $1 notepad.exe
	IfErrors EndReadMeBranch
		;MessageBox MB_YESNO|MB_ICONQUESTION "The installation process has been successfull.$\n$\nDo you want to see the README file now?" IDNO EndReadMeBranch
		;Exec '"$1" ${FILENAME}_readme.txt'
	EndReadMeBranch:

SectionEnd

; A LangString for the section name
LangString Sec2Name ${LANG_ENGLISH} "MP3Gain back end"
LangString Sec2Name ${LANG_HUNGARIAN} "MP3Gain háttér oldal"

Section !$(Sec2Name)

	SectionIn 1

	; Set output path to the installation directory.
	${SetOutPath} $INSTDIR
	
	; Put file there
	${File} "generated\" "mp3gain.exe"
SectionEnd

; A LangString for the section name
LangString Sec3Name ${LANG_ENGLISH} "QT4 runtime files"
LangString Sec3Name ${LANG_HUNGARIAN} "QT4 futásidejû fájlok"

Section !$(Sec3Name)

	SectionIn 1

	; Set output path to the installation directory.
	${SetOutPath} $INSTDIR
	
	; Put file there
	;File /r /x ${FILENAME}.exe /x mp3gain.exe "generated\*.*"
	${File} "generated\" "assistant.exe"
	${File} "generated\" "libgcc_s_dw2-1.dll"
	${File} "generated\" "mingwm10.dll"
	${File} "generated\" "phonon4.dll"
	${File} "generated\" "qsqlodbc4.dll"
	${File} "generated\" "QtCLucene4.dll"
	${File} "generated\" "QtCore4.dll"
	${File} "generated\" "QtGui4.dll"
	${File} "generated\" "QtHelp4.dll"
	${File} "generated\" "QtNetwork4.dll"
	${File} "generated\" "QtSql4.dll"
	${File} "generated\" "QtWebKit4.dll"
	${File} "generated\" "QtXml4.dll"
	${File} "generated\" "QtXmlPatterns4.dll"
	${SetOutPath} $INSTDIR\imageformats
	${File} "generated\imageformats\" "qgif4.dll"
	${File} "generated\imageformats\" "qjpeg4.dll"
	${SetOutPath} $INSTDIR\sqldrivers
	${File} "generated\sqldrivers\" "qsqlite4.dll"

SectionEnd

;--------------------------------

Section
	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip

		${CreateDirectory} $SMPROGRAMS\$R0
		${CreateShortCut} "$SMPROGRAMS\$R0\QMP3Gain.lnk" $INSTDIR\qmp3gain.exe
		${CreateShortCut} "$SMPROGRAMS\$R0\Uninstall QMP3Gain.lnk" $INSTDIR\${FILENAME}_uninstall.exe

		;SetShellVarContext All
		;CreateDirectory $SMPROGRAMS\$R0
		;CreateShortCut "$SMPROGRAMS\$R0\All users MakeNSISw.lnk" $INSTDIR\makensisw.exe

	skip:
SectionEnd

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
	
	; Remove registry keys
	DeleteRegKey HKLM SOFTWARE\Zematix\${NAME}

	; Remove uninstall keys
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"

	; Can't uninstall if uninstall log is missing!
	IfFileExists "$INSTDIR\${UninstLog}" +3
		MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
			Abort
 
	Push $R0
	Push $R1
	Push $R2
	SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
	FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
	StrCpy $R1 -1
 
	GetLineCount:
		ClearErrors
		FileRead $UninstLog $R0
		IntOp $R1 $R1 + 1
		StrCpy $R0 $R0 -2
		Push $R0   
		IfErrors 0 GetLineCount
 
	Pop $R0
 
	LoopRead:
		StrCmp $R1 0 LoopDone
		Pop $R0
 
		IfFileExists "$R0\*.*" 0 +3
			RMDir $R0  #is dir
		Goto +3
		IfFileExists $R0 0 +2
			Delete $R0 #is file
 
		IntOp $R1 $R1 - 1
		Goto LoopRead
	LoopDone:

	FileClose $UninstLog
	Delete "$INSTDIR\${UninstLog}"
	RMDir "$INSTDIR"
	Pop $R2
	Pop $R1
	Pop $R0
SectionEnd

;--------------------------------

; onInit messages

Function .onInit

	;Language selection dialog

	Push ""
	Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_HUNGARIAN}
	Push Hungarian
	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

	Pop $LANGUAGE
	StrCmp $LANGUAGE "cancel" 0 +2
		Abort

	;Message Dialog #1
	StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
		StrCpy $1 "This will install ${NAME} ${VERSION} on your computer.$\r$\nContinue?"
	StrCmp $LANGUAGE ${LANG_HUNGARIAN} 0 +2
		StrCpy $1 "${NAME} ${VERSION} alkalmazás telepítve lesz a számítógépére.$\r$\nFolytatja?"
	ClearErrors
	MessageBox MB_YESNO|MB_ICONQUESTION $1 IDYES ContinueInstall1
		Abort
	ContinueInstall1:

	;Message Dialog #2
	StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
		StrCpy $1 "This version named ${VERSION} is in beta phase and it is just for test purpose.$\r$\nBackup your data before launching ${NAME} application after the installation.$\r$\nContinue?"
	StrCmp $LANGUAGE ${LANG_HUNGARIAN} 0 +2
		StrCpy $1 "Ez a ${VERSION} verzió még báta fázisban van és csak tesztelési célra készült.$\r$\nMentse az adatait mielõtt elindítja a ${NAME} alkalmazást a sikeres telepítés után.$\r$\nFolytatja?"
	ClearErrors
	MessageBox MB_YESNO|MB_ICONQUESTION $1 IDYES ContinueInstall2
		Abort
	ContinueInstall2:

	ClearErrors
	;StrCpy $INSTDIR $1

FunctionEnd

; un.onInit messages

Function un.onInit
	; Read language used at installation
	ReadRegStr $LANGUAGE HKLM SOFTWARE\Zematix\${NAME} "Language" 
FunctionEnd
