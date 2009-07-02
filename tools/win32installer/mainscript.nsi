!include "MUI.nsh"

OutFile ../leechcraft-installer.exe
Name "LeechCraft"
SetCompressor /SOLID lzma
InstallDir "$PROGRAMFILES\Deviant\LeechCraft"
!define MUI_ABORTWARNING
!define MUI_ICON icon64.ico
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-colorful.ico"
#!define MUI_COMPONENTSPAGE_SMALLDESC

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Deviant\LeechCraft"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "LeechCraft"

!define MUI_WELCOMEFINISHPAGE_BITMAP installscreen.bmp
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var STARTMENU_FOLDER
!insertmacro MUI_PAGE_STARTMENU LeechCraft $STARTMENU_FOLDER
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\leechcraft.exe
!define MUI_FINISHPAGE_LINK "Web site"
!define MUI_FINISHPAGE_LINK_LOCATION http://leechcraft.org
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

InstType "Full"
InstType "Minimal"

SectionGroup "Core"
	Section "LeechCraft" MAINFILES
		SetOutPath $INSTDIR
		File plugininterface.dll
		File xmlsettingsdialog.dll
		File leechcraft.exe
		File icon64.ico
		File icon32.ico
		File icon24.ico
		File icon16.ico
		File /r icons
		File /r leechcraft
		File /r oxygen

		WriteRegStr HKCU "Software\Deviant\LeechCraft" "" $INSTDIR
		WriteUninstaller "$INSTDIR\Uninstall.exe"
	
		!insertmacro MUI_STARTMENU_WRITE_BEGIN LeechCraft
			CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Leechcraft.lnk" "$INSTDIR\leechcraft.exe" "" "$INSTDIR\icon64.ico"
			CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
		!insertmacro MUI_STARTMENU_WRITE_END

		SectionIn 1 2 RO
	SectionEnd

	Section "Qt" QT
		SetOutPath $INSTDIR
		File QtCore4.dll
		File QtGui4.dll
		File QtNetwork4.dll
		File QtScript4.dll
		File QtSql4.dll
		File QtSvg4.dll
		File QtWebkit4.dll
		File QtXml4.dll
		File qt.conf
		File phonon4.dll
#		File phonon_ds94.dll
		SetOutPath $INSTDIR\plugins
		File /r plugins\imageformats
		File /r plugins\sqldrivers
		SectionIn 1 2 RO
	SectionEnd

	Section "OpenSSL" OPENSSL
		SetOutPath $INSTDIR
		File libeay32.dll
		File ssleay32.dll
		SectionIn 1 2 RO
	SectionEnd

	Section "MSVC" MSVC
#		SetOutPath $WINDIR
#		File /r WinSxS
		SetOutPath $INSTDIR
		File vcredist_x86.exe
		DetailPrint "Installing Visual C++ 2008 Libraries"
		ExecWait '"$INSTDIR\vcredist_x86.exe" /q:a /c:"msiexec /i vcredist.msi /quiet"'
		SectionIn 1 2 RO
	SectionEnd
SectionGroupEnd

SectionGroup "Plugins"
	Section "Aggregator" AGGREGATORPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_aggregator.dll
		SectionIn 1
	SectionEnd
	Section "BitTorrent" TORRENTPLUGIN
		SetOutPath $INSTDIR
		File torrent.dll
		File boost_date_time-vc90-mt-1_37.dll
		File boost_filesystem-vc90-mt-1_37.dll
		File boost_system-vc90-mt-1_37.dll
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_bittorrent.dll
		SectionIn 1
	SectionEnd
	Section "CSTP" HTTPPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_cstp.dll
		SectionIn 1
	SectionEnd
	Section "DeadLyRiCS" DEADLYRICSPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_deadlyrics.dll
		SectionIn 1
	SectionEnd
	Section "HistoryHolder" HISTORYHOLDERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_historyholder.dll
		SectionIn 1
	SectionEnd
	Section "NetworkMonitor" NETWORKMONITORPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_networkmonitor.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku" POSHUKUPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku CleanWeb" POSHUKUCLEANWEBPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_cleanweb.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku FUA" POSHUKUFUAPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_fua.dll
		SectionIn 1
	SectionEnd
	Section "SeekThru" SEEKTHRUPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_seekthru.dll
		SectionIn 1
	SectionEnd
#	Section "LMP" LMPPLUGIN
#		SetOutPath $INSTDIR\plugins\bin
#		File plugins\bin\leechcraft_lmp.dll
#		SectionIn 1
#	SectionEnd
SectionGroupEnd

SectionGroup "Translations"
	Section "Arabic"
		SetOutPath $INSTDIR
		File /r *_ar.qm
		SectionIn 1
	SectionEnd
	Section "Italian"
		SetOutPath $INSTDIR
		File /r *_it.qm
		SectionIn 1
	SectionEnd
	Section "Russian"
		SetOutPath $INSTDIR
		File /r *_ru_RU.qm
		SectionIn 1
	SectionEnd
	Section "Ukrainian"
		SetOutPath $INSTDIR
		File /r *_uk_UA.qm
		SectionIn 1
	SectionEnd
SectionGroupEnd

Var MUI_TEMP

Section "Uninstall"
	RMDir /r "$INSTDIR"
		
	!insertmacro MUI_STARTMENU_GETFOLDER LeechCraft $MUI_TEMP
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
LangString DESC_QT ${LANG_ENGLISH} "Qt libraries."
LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual Studio libraries."
LangString DESC_OPENSSL ${LANG_ENGLISH} "OpenSSL libraries."
LangString DESC_HTTPPLUGIN ${LANG_ENGLISH} "Support for the HTTP protocol."
LangString DESC_TORRENTPLUGIN ${LANG_ENGLISH} "Feature-rich BitTorrent client."
LangString DESC_AGGREGATORPLUGIN ${LANG_ENGLISH} "Sophisticated RSS/Atom feed aggregator."
LangString DESC_HISTORYHOLDERPLUGIN ${LANG_ENGLISH} "Keeps history of downloaded files."
LangString DESC_NETWORKMONITORPLUGIN ${LANG_ENGLISH} "Monitors HTTP network requests."
LangString DESC_POSHUKUPLUGIN ${LANG_ENGLISH} "Full-featured web browser."
LangString DESC_POSHUKUCLEANWEBPLUGIN ${LANG_ENGLISH} "Ad blocker for the Poshuku."
LangString DESC_POSHUKUFUAPLUGIN ${LANG_ENGLISH} "Fake User Agent plugin for the Poshuku."
#LangString DESC_LMPPLUGIN ${LANG_ENGLISH} "LeechCraft Media Player."
LangString DESC_DEADLYRICSPLUGIN ${LANG_ENGLISH} "Fetches lyrics from LyricWiki.org."
LangString DESC_SEEKTHRUPLUGIN ${LANG_ENGLISH} "Client for OpenSearch-aware search engines."

LangString DESC_MAINFILES ${LANG_RUSSIAN} "Сам LeechCraft и его вспомогательные бИблиотеки."
LangString DESC_QT ${LANG_RUSSIAN} "Библиотеки Qt."
LangString DESC_MSVC ${LANG_RUSSIAN} "Библиотеки Microsoft Visual Studio."
LangString DESC_OPENSSL ${LANG_RUSSIAN} "Библиотеки OpenSSL."
LangString DESC_HTTPPLUGIN ${LANG_RUSSIAN} "Поддержка HTTP."
LangString DESC_TORRENTPLUGIN ${LANG_RUSSIAN} "Полнофункциональный Torrent-клиент."
LangString DESC_AGGREGATORPLUGIN ${LANG_RUSSIAN} "Агрегатор RSS/Atom-лент."
LangString DESC_HISTORYHOLDERPLUGIN ${LANG_RUSSIAN} "Хранит историю закачек."
LangString DESC_NETWORKMONITORPLUGIN ${LANG_RUSSIAN} "Следит за HTTP-запросами."
LangString DESC_POSHUKUPLUGIN ${LANG_RUSSIAN} "Веб-браузер."
LangString DESC_POSHUKUCLEANWEBPLUGIN ${LANG_RUSSIAN} "Блокировщик рекламы для Poshuku."
LangString DESC_POSHUKUFUAPLUGIN ${LANG_RUSSIAN} "Плагин для Poshuku, подделывающий идентификацию браузера."
#LangString DESC_LMPPLUGIN ${LANG_RUSSIAN} "LeechCraft Media Player."
LangString DESC_DEADLYRICSPLUGIN ${LANG_RUSSIAN} "Поиск песен на LyricWiki.org."
LangString DESC_SEEKTHRUPLUGIN ${LANG_RUSSIAN} "Клиент для поисковиков, поддерживающих OpenSearch."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MAINFILES} $(DESC_MAINFILES)
	!insertmacro MUI_DESCRIPTION_TEXT ${QT} $(DESC_QT)
	!insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
	!insertmacro MUI_DESCRIPTION_TEXT ${OPENSSL} $(DESC_OPENSSL)
	!insertmacro MUI_DESCRIPTION_TEXT ${HTTPPLUGIN} $(DESC_HTTPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${AGGREGATORPLUGIN} $(DESC_AGGREGATORPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${TORRENTPLUGIN} $(DESC_TORRENTPLUGIN)
#	!insertmacro MUI_DESCRIPTION_TEXT ${LMPPLUGIN} $(DESC_LMPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUPLUGIN} $(DESC_POSHUKUPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUFUAPLUGIN} $(DESC_POSHUKUFUAPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUCLEANWEBPLUGIN} $(DESC_POSHUKUCLEANWEBPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${DEADLYRICSPLUGIN} $(DESC_DEADLYRICSPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${SEEKTHRUPLUGIN} $(DESC_SEEKTHRUPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${HISTORYHOLDERPLUGIN} $(DESC_HISTORYHOLDERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${NETWORKMONITORPLUGIN} $(DESC_NETWORKMONITORPLUGIN)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
