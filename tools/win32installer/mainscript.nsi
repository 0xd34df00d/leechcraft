!include "MUI.nsh"

OutFile ../lcinstall-0.2.814.exe
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

Section "Main LeechCraft Files" MAINFILES
	SetOutPath $INSTDIR
	File plugininterface.dll
	File xmlsettingsdialog.dll
	File QtGui4.dll
	File QtNetwork4.dll
	File QtCore4.dll
	File QtXml4.dll
	File QtWebkit4.dll
	File QtSql4.dll
	File msvcp90.dll
	File msvcr90.dll
	File leechcraft.exe
	File icon64.ico
	File icon32.ico
	File icon24.ico
	File icon16.ico
	File /r icons
	
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
	Section "BitTorrent" TORRENTPLUGIN
		SetOutPath $INSTDIR
		File torrent.dll
		File boost_date_time-vc90-mt-1_36.dll
		File boost_filesystem-vc90-mt-1_36.dll
		File boost_thread-vc90-mt-1_36.dll
		File boost_system-vc90-mt-1_36.dll
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_torrent.dll
		SectionIn 1
	SectionEnd
	Section "Aggregator" AGGREGATORPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_aggregator.dll
		SectionIn 1
	SectionEnd
	Section "CSTP" HTTPPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_cstp.dll
		SectionIn 1
	SectionEnd
	Section "Remoter" REMOTERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_remoter.dll
		SectionIn 1
	SectionEnd
	Section "Batcher" BATCHERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_batcher.dll
		SectionIn 1
	SectionEnd
	Section "Chatter" CHATTERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File leechcraft_chatter.dll
		SectionIn 1
	SectionEnd
SectionGroupEnd

Var MUI_TEMP

Section "Uninstall"
	RMDir /r "$INSTDIR"
		
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

LangString DESC_MAINFILES ${LANG_ENGLISH} "LeechCraft executable and support libraries."
LangString DESC_HTTPPLUGIN ${LANG_ENGLISH} "A simple plugin implementing HTTP facilities."
LangString DESC_TORRENTPLUGIN ${LANG_ENGLISH} "A sophisticated feature-rich BitTorrent client."
LangString DESC_REMOTERPLUGIN ${LANG_ENGLISH} "Provides remote access to plugins supporting this feature."
LangString DESC_BATCHERPLUGIN ${LANG_ENGLISH} "Batch job manager."
LangString DESC_CHATTERPLUGIN ${LANG_ENGLISH} "IRC client."
LangString DESC_AGGREGATORPLUGIN ${LANG_ENGLISH} "RSS/Atom feed aggregator."

LangString DESC_MAINFILES ${LANG_RUSSIAN} "Сам LeechCraft и его вспомогательные бИблиотеки."
LangString DESC_HTTPPLUGIN ${LANG_RUSSIAN} "Простой HTTP-модуль."
LangString DESC_TORRENTPLUGIN ${LANG_RUSSIAN} "Полнофункциональный Torrent-клиент."
LangString DESC_REMOTERPLUGIN ${LANG_RUSSIAN} "Предоставляет возможность удаленного администрирования плагинов."
LangString DESC_BATCHERPLUGIN ${LANG_RUSSIAN} "Менеджер пакетных заданий."
LangString DESC_CHATTERPLUGIN ${LANG_RUSSIAN} "Клиент IRC."
LangString DESC_AGGREGATORPLUGIN ${LANG_RUSSIAN} "Агрегатор RSS/Atom-лент."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MAINFILES} $(DESC_MAINFILES)
	!insertmacro MUI_DESCRIPTION_TEXT ${HTTPPLUGIN} $(DESC_HTTPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${AGGREGATORPLUGIN} $(DESC_AGGREGATORPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${TORRENTPLUGIN} $(DESC_TORRENTPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${REMOTERPLUGIN} $(DESC_REMOTERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${BATCHERPLUGIN} $(DESC_BATCHERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${CHATTERPLUGIN} $(DESC_CHATTERPLUGIN)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
