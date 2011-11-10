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
!define MUI_HEADERIMAGE_BITMAP headerimage.bmp
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
		File icon64.ico
		File lcutil.dll
		File xmlsettingsdialog.dll
		File leechcraft.exe
		File boost_program_options-vc100-mt-1_47.dll
		File /r icons
		File /r leechcraft
		File /r oxygen
		SetOutPath $INSTDIR\settings
		File settings\coresettings.xml

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
		File Qt*4.dll
		File qt.conf
		File phonon4.dll
		SetOutPath $INSTDIR\plugins
		File /r plugins\imageformats
		File /r plugins\sqldrivers
		File /r plugins\phonon_backend
		SectionIn 1 2 RO
	SectionEnd

	Section "OpenSSL" OPENSSL
		SetOutPath $INSTDIR
		File libeay32.dll
		File ssleay32.dll
		SectionIn 1 2 RO
	SectionEnd

	Section "MSVC 2010 SP1 Redist" MSVC
		SetOutPath $TEMP
		File vcredist_x86.exe
		DetailPrint "Installing Visual C++ 2010 Libraries"
		ExecWait '"$TEMP\vcredist_x86.exe" /q /norestart"'

		SectionIn 1
	SectionEnd
SectionGroupEnd

SectionGroup "Plugins"
	Section "AdvancedNotifications" ANPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\advancednotificationssettings.xml

		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_advancednotifications.dll

		SetOutPath $INSTDIR\share\advancednotifications
		File /r share\qml\advancednotifications\*
		SetOutPath $INSTDIR\share\sounds
		File /r share\sounds\*
		SectionIn 1
	SectionEnd
	Section "Aggregator" AGGREGATORPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_aggregator.dll
		SetOutPath $INSTDIR\settings
		File settings\aggregatorsettings.xml
		SectionIn 1
	SectionEnd
	Section "Auscrie" AUSCRIEPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_auscrie.dll
		SectionIn 1
	SectionEnd
	Section "Azoth" AZOTHPLUGIN
        SetOutPath $INSTDIR\settings
        File settings\azoth*settings.xml
        File settings\azothsettings.xml

        SetOutPath $INSTDIR\plugins\bin
        File plugins\bin\leechcraft_azoth.dll
        File plugins\bin\leechcraft_azoth_*.dll

        SetOutPath $INSTDIR\share\azoth
        File /r share\azoth\*
        SectionIn 1
    SectionEnd
	Section "BitTorrent" TORRENTPLUGIN
		SetOutPath $INSTDIR
		File torrent.dll
		File boost_date_time-vc100-mt-1_47.dll
		File boost_filesystem-vc100-mt-1_47.dll
		File boost_system-vc100-mt-1_47.dll
		File boost_thread-vc100-mt-1_47.dll
		SetOutPath $INSTDIR\settings
		File settings\torrentsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_bittorrent.dll
		SectionIn 1
	SectionEnd
	Section "CSTP" HTTPPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\cstpsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_cstp.dll
		SectionIn 1
	SectionEnd
	Section "DeadLyrics" DEADLYRICSPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\deadlyricssettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_deadlyrics.dll
		SectionIn 1
	SectionEnd
	Section "Glance" GLANCEPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_glance.dll
		SectionIn 1
	SectionEnd
	Section "GMail Notifier" GMAILNOTIFIERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_gmailnotifier.dll
		SetOutPath $INSTDIR\settings
        File settings\gmailnotifiersettings.xml
		SectionIn 1
	SectionEnd
	Section "HistoryHolder" HISTORYHOLDERPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_historyholder.dll
		SectionIn 1
	SectionEnd
	Section "Kinotify" KINOTIFYPLUGIN
        SetOutPath $INSTDIR\settings
        File settings\kinotifysettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_kinotify.dll
        SetOutPath $INSTDIR\share\kinotify
        File /r share\kinotify\*
		SectionIn 1
	SectionEnd
	Section "LackMan" LACKMANPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\lackmansettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_lackman.dll
		SetOutPath $INSTDIR
		File 7za.exe
		File gunzip.exe
		SectionIn 1
	SectionEnd
	Section "NetworkMonitor" NETWORKMONITORPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_networkmonitor.dll
		SectionIn 1
	SectionEnd
	Section "New Life" NEWLIFEPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_newlife.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku" POSHUKUPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\poshukusettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku CleanWeb" POSHUKUCLEANWEBPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\poshukucleanwebsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_cleanweb.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku FatApe" POSHUKUFATAPEPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_fatape.dll
		SetOutPath $INSTDIR\settings
		File settings\poshukufatapesettings.xml
		SectionIn 1
	SectionEnd
	Section "Poshuku FileScheme" POSHUKUFILESCHEMEPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_filescheme.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku FUA" POSHUKUFUAPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\poshukufuasettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_fua.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku OnlineBookmarks" POSHUKUONLINEBOOKMARKS
		SetOutPath $INSTDIR
		File qjson.dll
		SetOutPath $INSTDIR\settings
		File settings\poshukuonlinebookmarkssettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_onlinebookmarks.dll
		SectionIn 1
	SectionEnd
	Section "Poshuku WYFV" POSHUKUWYFVPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\poshukuwyfvsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_poshuku_wyfv.dll
		SectionIn 1
	SectionEnd
	Section "SecMan" SECMANPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_secman.dll
		File plugins\bin\leechcraft_secman_simplestorage.dll
		SectionIn 1
	SectionEnd
	Section "SeekThru" SEEKTHRUPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\seekthrusettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_seekthru.dll
		SectionIn 1
	SectionEnd
	Section "ShellOpen" SHELLOPENPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_shellopen.dll
		SectionIn 1
	SectionEnd
	Section "Summary" SUMMARYPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_summary.dll
		SectionIn 1
	SectionEnd
	Section "Tab++" TABPPPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\tabppsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_tabpp.dll
		SectionIn 1
	SectionEnd
	Section "TabsList" TABSLISTPLUGIN
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_tabslist.dll
		SectionIn 1
	SectionEnd
	Section "vGrabber" VGRABBERPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\vgrabbersettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_vgrabber.dll
		SectionIn 1
	SectionEnd
SectionGroupEnd

SectionGroup "Translations"
	Section "Arabic"
		SetOutPath $INSTDIR
		File /r *_ar.qm
		SectionIn 1
	SectionEnd
	Section "German"
		SetOutPath $INSTDIR
		File /r *_de.qm
		SectionIn 1
	SectionEnd
	Section "Esperanto"
		SetOutPath $INSTDIR
		File /r *_eo.qm
		SectionIn 1
	SectionEnd
	Section "French"
		SetOutPath $INSTDIR
		File /r *_fr.qm
		SectionIn 1
	SectionEnd
	Section "Italian"
		SetOutPath $INSTDIR
		File /r *_it.qm
		SectionIn 1
	SectionEnd
	Section "Polish"
		SetOutPath $INSTDIR
		File /r *_pl.qm
		SectionIn 1
	SectionEnd
	Section "Russian"
		SetOutPath $INSTDIR
		File /r *_ru_RU.qm
		File /r *_ru.qm
		SectionIn 1
	SectionEnd
	Section "Spanish"
		SetOutPath $INSTDIR
		File /r *_es.qm
		SectionIn 1
	SectionEnd
	Section "Ukrainian"
		SetOutPath $INSTDIR
		File /r *_uk_UA.qm
		SectionIn 1
	SectionEnd
SectionGroupEnd

SectionGroup "Unsupported plugins"
#	Section "LCFTP" LCFTPPLUGIN
#		SetOutPath $INSTDIR\settings
#		File settings\lcftpsettings.xml
#		SetOutPath $INSTDIR
#		File libcurl.dll
#		SetOutPath $INSTDIR\plugins\bin
#		File plugins\bin\leechcraft_lcftp.dll
#	SectionEnd
	Section "LMP" LMPPLUGIN
		SetOutPath $INSTDIR\settings
		File settings\lmpsettings.xml
		SetOutPath $INSTDIR\plugins\bin
		File plugins\bin\leechcraft_lmp.dll
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

LangString DESC_MAINFILES ${LANG_ENGLISH} "LeechCraft Core."
LangString DESC_QT ${LANG_ENGLISH} "Qt libraries."
LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual Studio libraries."
LangString DESC_OPENSSL ${LANG_ENGLISH} "OpenSSL support."
LangString DESC_HTTPPLUGIN ${LANG_ENGLISH} "Support for the HTTP protocol."
LangString DESC_CHATTERPLUGIN ${LANG_ENGLISH} "IRC chat."
LangString DESC_TORRENTPLUGIN ${LANG_ENGLISH} "Feature-rich BitTorrent client."
LangString DESC_AGGREGATORPLUGIN ${LANG_ENGLISH} "Sophisticated RSS/Atom news feed reader."
LangString DESC_HISTORYHOLDERPLUGIN ${LANG_ENGLISH} "Keeps history of downloaded files."
LangString DESC_LCFTPPLUGIN ${LANG_ENGLISH} "FTP client."
LangString DESC_LMPPLUGIN ${LANG_ENGLISH} "LeechCraft Media Player."
LangString DESC_NETWORKMONITORPLUGIN ${LANG_ENGLISH} "Monitors HTTP network requests."
LangString DESC_NEWLIFEPLUGIN ${LANG_ENGLISH} "Imports settings from other programs."
LangString DESC_POSHUKUPLUGIN ${LANG_ENGLISH} "Full-featured web browser."
LangString DESC_POSHUKUCLEANWEBPLUGIN ${LANG_ENGLISH} "Ad blocker for the Poshuku compatible with Firefox's AdBlock lists."
LangString DESC_POSHUKUFILESCHEMEPLUGIN ${LANG_ENGLISH} "Support of the file:// scheme for the Poshuku."
LangString DESC_POSHUKUFUAPLUGIN ${LANG_ENGLISH} "Fake User Agent plugin for the Poshuku."
LangString DESC_POSHUKUWYFVPLUGIN ${LANG_ENGLISH} "Alternate media player for YouTube and other similar sites."
LangString DESC_DEADLYRICSPLUGIN ${LANG_ENGLISH} "Fetches lyrics from various sites."
LangString DESC_SEEKTHRUPLUGIN ${LANG_ENGLISH} "Client for OpenSearch-aware search engines."
LangString DESC_SUMMARYPLUGIN ${LANG_ENGLISH} "Summary tab with current downloads, events and status."
LangString DESC_VGRABBERPLUGIN ${LANG_ENGLISH} "VKontakte.ru audio/video grabber."
LangString DESC_AUSCRIEPLUGIN ${LANG_ENGLISH} "Auto Screenshooter."
LangString DESC_KINOTIFYPLUGIN ${LANG_ENGLISH} "Provides fancy notifications instead of plain default ones."
LangString DESC_TABPPPLUGIN ${LANG_ENGLISH} "Enhances tabbed navigation experience."
LangString DESC_LACKMANPLUGIN ${LANG_ENGLISH} "LeechCraft Package Manager allows one to install additional plugins, extensions, icons and various other data."
LangString DESC_SECMANPLUGIN ${LANG_ENGLISH} "Security Manager stores passwords and various other data."
LangString DESC_SHELLOPENPLUGIN ${LANG_ENGLISH} "Allows one to open unhandled entities via other applications."
LangString DESC_ANPLUGIN ${LANG_ENGLISH} "A highly flexible, powerful and configurable notifications framework."
LangString DESC_GLANCEPLUGIN ${LANG_ENGLISH} "Provides thumbnailed grid overview of tabs."
LangString DESC_GMAILNOTIFIERPLUGIN ${LANG_ENGLISH} "Notifies about new mail in your GMail inbox."
LangString DESC_POSHUKUFATAPEPLUGIN ${LANG_ENGLISH} "Adds support for GreaseMonkey userscripts."
LangString DESC_POSHUKUONLINEBOOKMARKSPLUGIN ${LANG_ENGLISH} "Allows one to synchronize bookmarks with services like Read It Later."
LangString DESC_TABSLISTPLUGIN ${LANG_ENGLISH} "Shows the list of currently opened tabs and allows one to quickly navigate between them."

LangString DESC_MAINFILES ${LANG_RUSSIAN} "Ядро LeechCraft."
LangString DESC_QT ${LANG_RUSSIAN} "Библиотеки Qt."
LangString DESC_MSVC ${LANG_RUSSIAN} "Библиотеки Microsoft Visual Studio."
LangString DESC_OPENSSL ${LANG_RUSSIAN} "Библиотеки OpenSSL."
LangString DESC_HTTPPLUGIN ${LANG_RUSSIAN} "Простой HTTP-клиент."
LangString DESC_CHATTERPLUGIN ${LANG_RUSSIAN} "Клиент для сетей IRC."
LangString DESC_TORRENTPLUGIN ${LANG_RUSSIAN} "Полнофункциональный Torrent-клиент."
LangString DESC_AGGREGATORPLUGIN ${LANG_RUSSIAN} "Клиент для чтения RSS/Atom-лент."
LangString DESC_HISTORYHOLDERPLUGIN ${LANG_RUSSIAN} "Хранение истории закачек."
LangString DESC_LCFTPPLUGIN ${LANG_RUSSIAN} "Двухпанельный FTP-клиент."
LangString DESC_LMPPLUGIN ${LANG_RUSSIAN} "LeechCraft Media Player - многофункциональный проигрыватель."
LangString DESC_NETWORKMONITORPLUGIN ${LANG_RUSSIAN} "Отображает HTTP-запросы."
LangString DESC_NEWLIFEPLUGIN ${LANG_RUSSIAN} "Импорт настроек из других программ."
LangString DESC_POSHUKUPLUGIN ${LANG_RUSSIAN} "Полнофункциональный веб-браузер."
LangString DESC_POSHUKUCLEANWEBPLUGIN ${LANG_RUSSIAN} "Блокировщик рекламы для Poshuku, совместимый с Firefox AdBlock."
LangString DESC_POSHUKUFILESCHEMEPLUGIN ${LANG_RUSSIAN} "Поддержка file://-схемы для Poshuku."
LangString DESC_POSHUKUFUAPLUGIN ${LANG_RUSSIAN} "Плагин для Poshuku, подделывающий идентификацию браузера."
LangString DESC_POSHUKUWYFVPLUGIN ${LANG_RUSSIAN} "Альтернативный медиа-проигрыватель для Youtube и подобных сайтов."
LangString DESC_DEADLYRICSPLUGIN ${LANG_RUSSIAN} "Клиент для поиска текстов песен."
LangString DESC_SEEKTHRUPLUGIN ${LANG_RUSSIAN} "Клиент для поисковиков, поддерживающих OpenSearch."
LangString DESC_SUMMARYPLUGIN ${LANG_RUSSIAN} "Сводка с текущими закачками, событиями и статусом."
LangString DESC_VGRABBERPLUGIN ${LANG_RUSSIAN} "Плагин для скачивания и проигрывания музыки и видео из социальной сети В Контакте."
LangString DESC_AUSCRIEPLUGIN ${LANG_RUSSIAN} "Плагин для создания и загрузки снимков окна LeechCraft в один клик."
LangString DESC_KINOTIFYPLUGIN ${LANG_RUSSIAN} "Kinotify предоставляет красивые уведомления вместо обычных уведомлений по умолчанию."
LangString DESC_TABPPPLUGIN ${LANG_RUSSIAN} "Tab++ улучшает работу с вкладками."
LangString DESC_LACKMANPLUGIN ${LANG_RUSSIAN} "LeechCraft Package Manager позволяет устанавливать дополнительные плагины, расширения, иконки и прочие данные."
LangString DESC_SECMANPLUGIN ${LANG_RUSSIAN} "Security Manager сохраняет пароли и прочую информацию."
LangString DESC_SHELLOPENPLUGIN ${LANG_RUSSIAN} "Позволяет открывать необработанные сущности другими приложениями."
LangString DESC_ANPLUGIN ${LANG_RUSSIAN} "Мощная и гибкая система уведомлений."
LangString DESC_GLANCEPLUGIN ${LANG_RUSSIAN} "Обеспечивает переключение между вкладками с предпросмотром их содержимого."
LangString DESC_GMAILNOTIFIERPLUGIN ${LANG_RUSSIAN} "Уведомляет о новой почте в вашей учетной записи GMail."
LangString DESC_POSHUKUFATAPEPLUGIN ${LANG_RUSSIAN} "Добавляет поддержку пользовательских скриптов GreaseMonkey."
LangString DESC_POSHUKUONLINEBOOKMARKSPLUGIN ${LANG_RUSSIAN} "Позволяет синхронизировать закладки с сервисами типа Read It Later."
LangString DESC_TABSLISTPLUGIN ${LANG_ENGLISH} "Показывает список открытых вкладок и позволяет легко перемещаться между ними."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MAINFILES} $(DESC_MAINFILES)
	!insertmacro MUI_DESCRIPTION_TEXT ${QT} $(DESC_QT)
	!insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
	!insertmacro MUI_DESCRIPTION_TEXT ${OPENSSL} $(DESC_OPENSSL)
	!insertmacro MUI_DESCRIPTION_TEXT ${HTTPPLUGIN} $(DESC_HTTPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${AGGREGATORPLUGIN} $(DESC_AGGREGATORPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${TORRENTPLUGIN} $(DESC_TORRENTPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${LMPPLUGIN} $(DESC_LMPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${NEWLIFEPLUGIN} $(DESC_NEWLIFEPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUPLUGIN} $(DESC_POSHUKUPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUFILESCHEMEPLUGIN} $(DESC_POSHUKUFILESCHEMEPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUFUAPLUGIN} $(DESC_POSHUKUFUAPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUCLEANWEBPLUGIN} $(DESC_POSHUKUCLEANWEBPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUWYFVPLUGIN} $(DESC_POSHUKUWYFVPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${DEADLYRICSPLUGIN} $(DESC_DEADLYRICSPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${SEEKTHRUPLUGIN} $(DESC_SEEKTHRUPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${HISTORYHOLDERPLUGIN} $(DESC_HISTORYHOLDERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${NETWORKMONITORPLUGIN} $(DESC_NETWORKMONITORPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${CHATTERPLUGIN} $(DESC_CHATTERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${LCFTPPLUGIN} $(DESC_LCFTPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${VGRABBERPLUGIN} $(DESC_VGRABBERPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${SUMMARYPLUGIN} $(DESC_SUMMARYPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${AUSCRIEPLUGIN} $(DESC_AUSCRIEPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${KINOTIFYPLUGIN} $(DESC_KINOTIFYPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${TABPPPLUGIN} $(DESC_TABPPPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${LACKMANPLUGIN} $(DESC_LACKMANPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${SECMANPLUGIN} $(DESC_SECMANPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${SHELLOPENPLUGIN} $(DESC_SHELLOPENPLUGIN)
	!insertmacro MUI_DESCRIPTION_TEXT ${ANPLUGIN} ${DESC_ANPLUGIN}
	!insertmacro MUI_DESCRIPTION_TEXT ${GLANCEPLUGIN} ${DESC_GLANCEPLUGIN}
	!insertmacro MUI_DESCRIPTION_TEXT ${GMAILNOTIFIERPLUGIN} ${DESC_GMAILNOTIFIERPLUGIN}
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUFATAPEPLUGIN} ${DESC_POSHUKUFATAPEPLUGIN}
	!insertmacro MUI_DESCRIPTION_TEXT ${POSHUKUONLINEBOOKMARKSPLUGIN} ${DESC_POSHUKUONLINEBOOKMARKSPLUGIN}
	!insertmacro MUI_DESCRIPTION_TEXT ${TABSLISTPLUGIN} ${DESC_TABSLISTPLUGIN}
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd
