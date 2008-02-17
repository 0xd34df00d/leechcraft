!include "MUI.nsh"

OutFile ../lcinstall-pb8.exe
Name "Deviant LeechCraft"
SetCompressor /SOLID lzma
InstallDir "$PROGRAMFILES\Deviant\LeechCraft"
!define MUI_ABORTWARNING
!define MUI_ICON icon32.ico
!define MUI_UNICON icon32.ico
#!define MUI_COMPONENTSPAGE_SMALLDESC

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Deviant\LeechCraft"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Deviant\LeechCraft"

!define MUI_WELCOMEFINISHPAGE_BITMAP installscreen.bmp
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var STARTMENU_FOLDER
!insertmacro MUI_PAGE_STARTMENU Deviant $STARTMENU_FOLDER
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\leechcraft.exe
!define MUI_FINISHPAGE_LINK "Bugtracker"
!define MUI_FINISHPAGE_LINK_LOCATION http://bugs.deviant-soft.ws
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

InstType "Full"
InstType "Minimal"

Section "Qt4 Runtime" QT4RUNTIME
	SetOutPath $INSTDIR
	File QtGui4.dll
	File QtNetwork4.dll
	File QtCore4.dll
	File QtXml4.dll
	SectionIn 1 2
SectionEnd

Section "Main LeechCraft Files" MAINFILES
	SetOutPath $INSTDIR
	File libexceptions.dll
	File libplugininterface.dll
	File libsettingsdialog.dll
	File libxmlsettingsdialog.dll
	File mingwm10.dll
#	File msvcr80.dll
	File leechcraft.exe
	File icon64.ico
	File icon32.ico
	File icon24.ico
	File icon16.ico
	
	WriteRegStr HKCU "Software\Deviant\LeechCraft" "" $INSTDIR
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Deviant
		CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Leechcraft.lnk" "$INSTDIR\leechcraft.exe" "" "$INSTDIR\icon16.ico"
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	!insertmacro MUI_STARTMENU_WRITE_END
	
	SectionIn 1 2
SectionEnd

SectionGroup "Plugins"
	Section "HTTP/FTP" HTTPPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_http.dll
		SectionIn 1
	SectionEnd
	Section "BitTorrent" TORRENTPLUGIN
		SetOutPath $INSTDIR
		File torrent.dll
		File boost_date_time-mgw42-mt-1_34_1.dll
		File boost_filesystem-mgw42-mt-1_34_1.dll
		File boost_thread-mgw42-mt-1_34_1.dll
		File zlib1.dll
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_torrent.dll
		SectionIn 1
	SectionEnd
	Section "Remoter" REMOTERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_remoter.dll
		SectionIn 1
	SectionEnd
	Section "Batcher" BATCHERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_batcher.dll
		SectionIn 1
	SectionEnd
	Section "Cron" CRONPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_cron.dll
		SectionIn 1
	SectionEnd
	Section "MailLeecher" MAILPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File libleechcraft_mailleecher.dll
		SectionIn 1
	SectionEnd
SectionGroupEnd

Var MUI_TEMP

Section "Uninstall"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\libexceptions.dll"
	Delete "$INSTDIR\libplugininterface.dll"
	Delete "$INSTDIR\libsettingsdialog.dll"
	Delete "$INSTDIR\leechcraft.exe"
	Delete "$INSTDIR\QtCore4.dll"
	Delete "$INSTDIR\QtNetwork4.dll"
	Delete "$INSTDIR\QtGui4.dll"
	Delete "$INSTDIR\QtXml4.dll"
	Delete "$INSTDIR\mingwm10.dll"
	Delete "$INSTDIR\torrent.dll"
	Delete "$INSTDIR\zlib1.dll"
	Delete "$INSTDIR\boost_date_time-mgw42-mt-1_34_1.dll"
	Delete "$INSTDIR\boost_filesystem-mgw42-mt-1_34_1.dll"
	Delete "$INSTDIR\boost_thread-mgw42-mt-1_34_1.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_http.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_torrent.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_remoter.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_batcher.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_cron.dll"
	Delete "$INSTDIR\plugins\bin\libleechcraft_mailleecher.dll"
	Delete "$INSTDIR\plugins\bin\warning.log"
	Delete "$INSTDIR\plugins\bin\debug.log"
	Delete "$INSTDIR\plugins\bin\critical.log"
	Delete "$INSTDIR\plugins\bin\fatal.log"
	Delete "$INSTDIR\warning.log"
	Delete "$INSTDIR\debug.log"
	Delete "$INSTDIR\critical.log"
	Delete "$INSTDIR\fatal.log"
	Delete "$INSTDIR\icon64.ico"
	Delete "$INSTDIR\icon32.ico"
	Delete "$INSTDIR\icon24.ico"
	Delete "$INSTDIR\icon16.ico"

	RMDir "$INSTDIR\plugins\bin"
	RMDir "$INSTDIR\plugins"
	RMDir "$INSTDIR"
		
	!insertmacro MUI_STARTMENU_GETFOLDER Deviant $MUI_TEMP
	Delete "$SMPROGRAMS\$MUI_TEMP\Leechcraft.lnk"
	Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
	
	StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
	startMenuDeleteLoop:
		ClearErrors
		RMDir $MUI_TEMP
		GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
		IfErrors startMenuDeleteLoopDone
  
		StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
	startMenuDeleteLoopDone:
	DeleteRegKey /ifempty HKCU "Software\Deviant\LeechCraft"
SectionEnd

LangString DESC_QT4RUNTIME ${LANG_ENGLISH} "Qt4 library core files and MinGW support library."
LangString DESC_MAINFILES ${LANG_ENGLISH} "LeechCraft executable and support libraries."
LangString DESC_HTTPPLUGIN ${LANG_ENGLISH} "A simple plugin implementing HTTP/FTP facilities."
LangString DESC_TORRENTPLUGIN ${LANG_ENGLISH} "A simple plugin implementing BitTorrent protocol."
LangString DESC_REMOTERPLUGIN ${LANG_ENGLISH} "Provides remote access to plugins supporting this feature."
LangString DESC_BATCHERPLUGIN ${LANG_ENGLISH} "Batch job manager."
LangString DESC_CRONPLUGIN ${LANG_ENGLISH} "Job scheduler."
LangString DESC_MAILPLUGIN ${LANG_ENGLISH} "POP3 mail backuper."

LangString DESC_QT4RUNTIME ${LANG_RUSSIAN} "Библиотеки Qt4."
LangString DESC_MAINFILES ${LANG_RUSSIAN} "Сам LeechCraft и его вспомогательные бИблиотеки."
LangString DESC_HTTPPLUGIN ${LANG_RUSSIAN} "Простой HTTP/FTP-модуль."
LangString DESC_TORRENTPLUGIN ${LANG_RUSSIAN} "Простейший Torrent-клиент."
LangString DESC_REMOTERPLUGIN ${LANG_RUSSIAN} "Предоставляет возможность удаленного администрирования плагинов."
LangString DESC_BATCHERPLUGIN ${LANG_RUSSIAN} "Менеджер пакетных заданий."
LangString DESC_CRONPLUGIN ${LANG_RUSSIAN} "Планировщик заданий."
LangString DESC_MAILPLUGIN ${LANG_RUSSIAN} "Выкачиватель почтовых ящиков по POP3."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${QT4RUNTIME} $(DESC_QT4RUNTIME)
	!insertmacro MUI_DESCRIPTION_TEXT ${MAINFILES} $(DESC_MAINFILES)
	!insertmacro MUI_DESCRIPTION_TEXT ${HTTPPLUGIN} $(DESC_HTTPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${TORRENTPLUGIN} $(DESC_TORRENTPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${REMOTERPLUGIN} $(DESC_REMOTERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${BATCHERPLUGIN} $(DESC_BATCHERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${CRONPLUGIN} $(DESC_CRONPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${MAILPLUGIN} $(DESC_MAILPLUGIN)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
