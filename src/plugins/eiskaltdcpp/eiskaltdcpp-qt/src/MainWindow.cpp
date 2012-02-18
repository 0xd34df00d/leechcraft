/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"

#include "dcpp/forward.h"
#include "dcpp/QueueManager.h"
#include "dcpp/HashManager.h"
#include "dcpp/Thread.h"
#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#ifdef USE_JS
#include "ScriptEngine.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QRegExp>
#include <QObject>

#ifdef DBUS_NOTIFY
#include <QtDBus>
#endif
#include "MainWindow.h"
#include "Notification.h"

#include <stdlib.h>
#include <string>
#include <iostream>

#include <QPainter>
#include <QPushButton>
#include <QSize>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QtDebug>
#include <QTextCodec>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QProgressBar>
#include <QFileDialog>
#include <QRegExp>
#include <QDir>
#include <QInputDialog>

#include "HubFrame.h"
#include "HubManager.h"
#include "HashProgress.h"
#include "PMWindow.h"
#include "TransferView.h"
#include "ShareBrowser.h"
#include "QuickConnect.h"
#include "SearchFrame.h"
#include "ADLS.h"
#include "Settings.h"
#include "FavoriteHubs.h"
#include "PublicHubs.h"
#include "FavoriteUsers.h"
#include "DownloadQueue.h"
#include "FinishedTransfers.h"
#include "AntiSpamFrame.h"
#include "IPFilterFrame.h"
#include "ToolBar.h"
#include "Magnet.h"
#include "FileHasher.h"
#include "SpyFrame.h"
#include "SideBar.h"
#include "ActionCustomizer.h"
#include "MultiLineToolBar.h"
#include "IPFilter.h"
#include "SearchBlacklist.h"
#include "QueuedUsers.h"
#ifdef FREE_SPACE_BAR_C
#include "extra/freespace.h"
#endif
#ifdef USE_JS
#include "ScriptManagerDialog.h"
#include "scriptengine/ScriptConsole.h"
#endif

#include <interfaces/core/icoreproxy.h>

#include "dcpp/ShareManager.h"
#include "dcpp/ConnectivityManager.h"
#include "dcpp/Singleton.h"
#include "dcpp/SettingsManager.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

#ifndef Q_WS_WIN
#include <unistd.h>
#include <signal.h>
#ifndef __HAIKU__
#include <execinfo.h>

#ifdef ENABLE_STACKTRACE
#include "extra/stacktrace.h"
#endif // ENABLE_STACKTRACE

void installHandlers();
#endif

#ifdef FORCE_XDG
#include <QTextStream>
void migrateConfig();
#endif

#else//WIN32
#include <locale.h>
#endif

using namespace std;

static const QString &TOOLBUTTON_STYLE = "mainwindow/toolbar-toolbutton-style";
static const QString &EMPTY_SETTINGS = "mainwindow/empty-settings";
static const QString &SIDEBAR_SHOW_CLOSEBUTTONS = "mainwindow/sidebar-with-close-buttons";

void callBack(void* x, const std::string& a)
{
    std::cout << QObject::tr("Loading: ").toStdString() << a << std::endl;
}

void parseCmdLine(const QStringList &);

void MainLayout::Init(ICoreProxy_ptr proxy){
    Q_UNUSED(proxy);

    MainLayout::instance = this;

    dcpp::startup(callBack, NULL);
    dcpp::TimerManager::getInstance()->start();

    HashManager::getInstance()->setPriority(Thread::IDLE);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    WulforSettings::newInstance();
    WulforSettings::getInstance()->load();
    WulforSettings::getInstance()->loadTranslation();
    WulforSettings::getInstance()->loadTheme();

    WulforUtil::newInstance();

    Text::hubDefaultCharset = WulforUtil::getInstance()->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE)).toStdString();

    if (WulforUtil::getInstance()->loadUserIcons())
        std::cout << QObject::tr("UserList icons has been loaded").toStdString() << std::endl;

    if (WulforUtil::getInstance()->loadIcons())
        std::cout << QObject::tr("Application icons has been loaded").toStdString() << std::endl;

    setUnload(!WBGET(WB_TRAY_ENABLED));

    HubManager::newInstance();

    WulforSettings::getInstance()->loadTheme();

    if (WBGET(WB_APP_ENABLE_EMOTICON)){
        EmoticonFactory::newInstance();
        EmoticonFactory::getInstance()->load();
    }

#ifdef USE_ASPELL
    if (WBGET(WB_APP_ENABLE_ASPELL))
        SpellCheck::newInstance();
#endif

    Notification::newInstance();

#ifdef USE_JS
    ScriptEngine::newInstance();
#endif

    FinishedUploads::newInstance();
    FinishedDownloads::newInstance();
    QueuedUsers::newInstance();

    _lc_MW = proxy->GetMainWindow();
}

void MainLayout::SecondInit(){
    exitBegin = false;

    if (WBGET(WB_ANTISPAM_ENABLED)){
        AntiSpam::newInstance();

        AntiSpam::getInstance()->loadLists();
        AntiSpam::getInstance()->loadSettings();
    }

    if (WBGET(WB_IPFILTER_ENABLED)){
        IPFilter::newInstance();

        IPFilter::getInstance()->loadList();
    }

    ShortcutManager::newInstance();

    init();

    retranslateUi();

    LogManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);

    startSocket(false);

    setStatusMessage(tr("Ready"));

    TransferView::newInstance();

    transfer_dock->setWidget(TransferView::getInstance());
    toolsTransfers->setChecked(transfer_dock->isVisible());

    QFont f;

    if (!WSGET(WS_APP_FONT).isEmpty() && f.fromString(WSGET(WS_APP_FONT)))
        qApp->setFont(f);

    if (!WSGET(WS_APP_THEME).isEmpty())
        qApp->setStyle(WSGET(WS_APP_THEME));

    if (WBGET(WB_APP_REMOVE_NOT_EX_DIRS)){
        StringPairList directories = ShareManager::getInstance()->getDirectories();
        for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
            QDir dir(_q(it->second));

            if (!dir.exists()){
                try {
                    ShareManager::getInstance()->removeDirectory(it->second);
                }
                catch (const std::exception&){}
            }
        }
    }

    autoconnect();
}

QByteArray MainLayout::GetUniqueID() const {
    return "org.EiskaltDCPP";
}

QString MainLayout::GetName () const {
    return "EiskaltDC++";
}

QString MainLayout::GetInfo() const {
    return "Direct Connect client";
}

void MainLayout::Release(){
    setUnload(true);
    exitBegin = true;

    if (TransferView::getInstance()){
        TransferView::getInstance()->close();
        TransferView::deleteInstance();
    }

    if (SearchManager::getInstance())
        SearchManager::getInstance()->disconnect();

    if (ConnectionManager::getInstance())
        ConnectionManager::getInstance()->disconnect();

    QMap< ArenaWidget*, QWidget* > map = arenaMap;
    QMap< ArenaWidget*, QWidget* >::iterator it = map.begin();

    for(; it != map.end(); ++it){
        dcpp::ISingleton *sg = dynamic_cast< dcpp::ISingleton* >(it.value());

        if (sg && sg != dynamic_cast< dcpp::ISingleton* >(HubManager::getInstance())){
            it.key()->setUnload(true);
            it.value()->close();

            sg->release();

            continue;
        }
        else if (sg == dynamic_cast< dcpp::ISingleton* >(HubManager::getInstance()))
            continue;//Do not remove HubManager

        if (arenaMap.contains(it.key()))//some widgets can autodelete itself from arena widgets
            it.value()->close();
    }

    if (HubManager::getInstance()){
        HubManager::getInstance()->setUnload(true);
        HubManager::getInstance()->close();

        HubManager::getInstance()->release();
    }

    LogManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    QueueManager::getInstance()->removeListener(this);

    if (AntiSpam::getInstance()){
        AntiSpam::getInstance()->saveLists();
        AntiSpam::getInstance()->saveSettings();
        AntiSpam::deleteInstance();
    }

    delete arena;
    delete fBar;
    delete sBar;

    ShortcutManager::deleteInstance();
    SearchBlacklist::deleteInstance();
}

QIcon MainLayout::GetIcon() const{
    return QIcon();
}

LeechCraft::TabClasses_t MainLayout::GetTabClasses () const {
    static LeechCraft::TabClassInfo tinfo[ArenaWidget::NoRole] = {
                                                {"DCPPHubFrame",            "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPHubManger",           "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPPrivateMessage",      "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPShareBrowser",        "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPFavoriteHubs",        "DC++ Favorite Hubs",       "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPFavoriteUsers",       "DC++ Favorite Users",      "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPSearchFrame",         "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPAdls",                "DC++ ADLS",                "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPPublicHubs",          "DC++ Public Hubs",         "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPDownloads",           "DC++ Downloads",           "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPFinishedUploads",     "DC++ Finished Uploads",    "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPFinishedDownloads",   "DC++ Finished Downloads",  "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPSpy",                 "DC++ Spy",                 "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)},
                                                {"DCPPCustomWidget",        "EiskaltDC++",              "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)LeechCraft::TFSingle},
                                                {"DCPPQueuedusers",         "DC++ Queued Users",        "EiskaltDC++ Tab", QIcon(), 60, (LeechCraft::TabFeatures)(LeechCraft::TFSingle | LeechCraft::TFOpenableByRequest)}
                                              };
    static QList<LeechCraft::TabClassInfo> classes;

    if (classes.isEmpty()){
        for (int i = 0; i < (int)ArenaWidget::NoRole; i++)
            classes.push_back(tinfo[i]);
    }

    return classes;
}

void MainLayout::TabOpenRequested (const QByteArray &arr){
    QStringList classes;

    classes     << "DCPPHubFrame"
                << "DCPPHubManger"
                << "DCPPPrivateMessage"
                << "DCPPShareBrowser"
                << "DCPPFavoriteHubs"
                << "DCPPFavoriteUsers"
                << "DCPPSearchFrame"
                << "DCPPAdls"
                << "DCPPPublicHubs"
                << "DCPPDownloads"
                << "DCPPFinishedUploads"
                << "DCPPFinishedDownloads"
                << "DCPPSpy"
                << "DCPPCustomWidget"
                << "DCPPQueuedusers";

    if (!classes.contains(QString(arr)))
        return;

    ArenaWidget::Role role = static_cast<ArenaWidget::Role>(classes.indexOf(QString(arr)));
    ArenaWidget *awgt = widgetForRole(role);

    if (awgt){
        mapWidgetOnArena(awgt);
        addArenaWidgetOnToolbar(awgt, false);
    }
}

MainLayout::MainLayout (QWidget *parent):
        statusLabel(NULL),
        tBar(NULL),
        mBar(NULL),
        fBar(NULL),
        sBar(NULL),
        _progress_dialog(NULL),
        sideDock(NULL),
        sideTree(NULL),
        menuPanels(NULL),
        wcontainer(NULL)

{

}

HashProgress* MainLayout::progress_dialog() {
    if( _progress_dialog == NULL ) {
        _progress_dialog = new HashProgress(this);
        //qDebug("Lazy initializtion of progress_dialog");
    }
    return _progress_dialog;
}

MainLayout::~MainLayout(){
}

void MainLayout::closeEvent(QCloseEvent *c_e){
    c_e->accept();
}

void MainLayout::beginExit(){
    exitBegin = true;
    setUnload(true);
}

void MainLayout::show(){
    if (showMax)
        showMaximized();
    else
        showNormal();
}

void MainLayout::showEvent(QShowEvent *e){
    if (!showMax && w > 0 && h > 0 && w != width() && h != height())
        this->resize(QSize(w, h));

    if (WBGET(WB_APP_AUTO_AWAY) && !Util::getManualAway()){
        Util::setAway(false);

        toolsAwayOff->setChecked(true);
    }

    if (transfer_dock->isVisible())
        toolsTransfers->setChecked(true);

    if (sideDock)
        sideDock->setVisible(panelsWidgets->isChecked());

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(arena->widget());

    if (!awgt)
        return;

    ArenaWidget::Role role = awgt->role();

    bool widgetWithFilter = role == ArenaWidget::Hub ||
                            role == ArenaWidget::PrivateMessage ||
                            role == ArenaWidget::ShareBrowser ||
                            role == ArenaWidget::PublicHubs ||
                            role == ArenaWidget::Search;

    chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
    findInWidget->setEnabled(widgetWithFilter);
    chatDisable->setEnabled(role == ArenaWidget::Hub);

    if (_q(SETTING(NICK)).isEmpty()){
        activateWindow();
        raise();

        bool ok = false;
        QString new_nick = QInputDialog::getText(this, tr("Enter user nick"), tr("Nick"), QLineEdit::Normal, tr("User"), &ok);

        if (ok && !new_nick.isEmpty()){
            SettingsManager::getInstance()->set(SettingsManager::NICK, _tq(new_nick));

            ok = (QMessageBox::question(this, "EiskaltDC++", tr("Would you like to change other settings?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No);
        }

        if (!ok)
            slotToolsSettings();
    }

    e->accept();
}

void MainLayout::hideEvent(QHideEvent *e){
    showMax = isMaximized();

    if (!showMax){
        h = height();
        w = width();
        xPos = x();
        yPos = y();
    }

    e->accept();

    if (sideDock && sideDock->isFloating())
        sideDock->hide();

    if (WBGET(WB_APP_AUTO_AWAY)){
        Util::setAway(true);

        toolsAwayOn->setChecked(true);
    }
}

bool MainLayout::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::WindowActivate) {
        redrawToolPanel();
    }
    else if( obj == progressHashing && e->type() == QEvent::MouseButtonDblClick ) {
        slotFileHashProgress();

        return true;
    }
    else if (obj == progressSpace && e->type() == QEvent::MouseButtonDblClick ){
        slotFileOpenDownloadDirectory();

        return true;
    }
    else if (obj == sideTree && sideTree && e->type() == QEvent::Resize) {
        if (WBGET(SIDEBAR_SHOW_CLOSEBUTTONS, true)){
            sideTree->header()->resizeSection(0, sideTree->contentsRect().width() - 20);
            sideTree->header()->resizeSection(1, 18);
        }
        else{
            sideTree->header()->resizeSection(0, sideTree->contentsRect().width());
            sideTree->header()->resizeSection(1, 0);
        }
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainLayout::init(){
    setObjectName("MainWindow");

    connect(this, SIGNAL(coreLogMessage(QString)), this, SLOT(setStatusMessage(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreOpenShare(dcpp::UserPtr,QString,QString)), this, SLOT(showShareBrowser(dcpp::UserPtr,QString,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUpdateStats(QMap<QString,QString>)), this, SLOT(updateStatus(QMap<QString,QString>)), Qt::QueuedConnection);

    arena = new QDockWidget();
#if QT_VERSION >= 0x040500
    arena->setWidget(NULL);
    arena->setFloating(false);
#endif
    arena->setContentsMargins(0, 0, 0, 0);
    arena->setAllowedAreas(Qt::RightDockWidgetArea);
    arena->setFeatures(QDockWidget::NoDockWidgetFeatures);
    arena->setContextMenuPolicy(Qt::CustomContextMenu);
    arena->setTitleBarWidget(new QWidget(arena));
    arena->setMinimumSize( 10, 10 );

    transfer_dock = new QDockWidget(this);
#if QT_VERSION >= 0x040500
    transfer_dock->setWidget(NULL);
    transfer_dock->setFloating(false);
#endif
    transfer_dock->setObjectName("transfer_dock");
    transfer_dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    transfer_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    transfer_dock->setContextMenuPolicy(Qt::CustomContextMenu);
    transfer_dock->setTitleBarWidget(new QWidget(transfer_dock));
    transfer_dock->setMinimumSize(QSize(8, 8));

    //setCentralWidget(arena);
    //addDockWidget(Qt::RightDockWidgetArea, arena);
    _lc_MW->addDockWidget(Qt::BottomDockWidgetArea, transfer_dock);

    transfer_dock->hide();

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    setWindowIcon(WICON(WulforUtil::eiICON_APPL));

    setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));

    initActions();

    initMenuBar();

    initStatusBar();

    initSearchBar();

    initToolbar();

    initSideBar();

    loadSettings();

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotExit()));

#ifdef LUA_SCRIPT
    ScriptManager::getInstance()->load();
    if (BOOLSETTING(USE_LUA)){
        // Start as late as possible, as we might (formatting.lua) need to examine settings
        string defaultluascript="startup.lua";
        ScriptManager::getInstance()->EvaluateFile(defaultluascript);
    }
#endif

    totalDown = WSGET(WS_APP_TOTAL_DOWN).toULongLong();
    totalUp =  WSGET(WS_APP_TOTAL_UP).toULongLong();
}

void MainLayout::loadSettings(){
    WulforSettings *WS = WulforSettings::getInstance();

    showMax = WS->getBool(WB_MAINWINDOW_MAXIMIZED);
    w = WS->getInt(WI_MAINWINDOW_WIDTH);
    h = WS->getInt(WI_MAINWINDOW_HEIGHT);
    xPos = WS->getInt(WI_MAINWINDOW_X);
    yPos = WS->getInt(WI_MAINWINDOW_Y);

    QPoint p(xPos, yPos);
    QSize  sz(w, h);

    if (p.x() >= 0 && p.y() >= 0)
        this->move(p);

    if (sz.width() > 0 && sz.height() > 0)
        this->resize(sz);

    QString wstate = WSGET(WS_MAINWINDOW_STATE);

    if (!wstate.isEmpty())
        this->restoreState(QByteArray::fromBase64(wstate.toAscii()));

    fBar->setVisible(WBGET(WB_TOOLS_PANEL_VISIBLE));
    panelsTools->setChecked(WBGET(WB_TOOLS_PANEL_VISIBLE));

    sBar->setVisible(WBGET(WB_SEARCH_PANEL_VISIBLE));
    panelsSearch->setChecked(WBGET(WB_SEARCH_PANEL_VISIBLE));

    if (sideDock){
        if (sideDock->isFloating() && WBGET(WB_MAINWINDOW_HIDE) && WBGET(WB_TRAY_ENABLED))
            sideDock->hide();
        else
            sideDock->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
    }
    else if (mBar)
        mBar->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));
    else if (tBar)
        tBar->setVisible(WBGET(WB_WIDGETS_PANEL_VISIBLE));

    panelsWidgets->setChecked(WBGET(WB_WIDGETS_PANEL_VISIBLE));

    if (!WBGET(WB_MAIN_MENU_VISIBLE))
        toggleMainMenu(false);

    if (WBGET("mainwindow/dont-show-icons-in-menus", false))
        qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
}

void MainLayout::saveSettings(){
    static bool stateIsSaved = false;

    if (stateIsSaved)
        return;

    if (isVisible())
        showMax = isMaximized();

    WBSET(WB_MAINWINDOW_MAXIMIZED, showMax);

    if (!showMax && height() > 0 && width() > 0){
        WISET(WI_MAINWINDOW_HEIGHT, height());
        WISET(WI_MAINWINDOW_WIDTH, width());
    }
    else{
        WISET(WI_MAINWINDOW_HEIGHT, h);
        WISET(WI_MAINWINDOW_WIDTH, w);
    }

    if (!showMax && x() >= 0 && y() >= 0){
        WISET(WI_MAINWINDOW_X, x());
        WISET(WI_MAINWINDOW_Y, y());
    }
    else{
        WISET(WI_MAINWINDOW_X, xPos);
        WISET(WI_MAINWINDOW_Y, yPos);
    }

    if (WBGET(WB_MAINWINDOW_REMEMBER))
        WBSET(WB_MAINWINDOW_HIDE, !isVisible());

    WSSET(WS_MAINWINDOW_STATE, saveState().toBase64());
    WSSET(WS_APP_TOTAL_DOWN, QString().setNum(totalDown));
    WSSET(WS_APP_TOTAL_UP, QString().setNum(totalUp));

    stateIsSaved = true;
}

void MainLayout::initActions(){

    WulforUtil *WU = WulforUtil::getInstance();
    ShortcutManager *SM = ShortcutManager::getInstance();

    {
        fileOpenMagnet = new QAction("", this);
        fileOpenMagnet->setObjectName("fileOpenMagnet");
        SM->registerShortcut(fileOpenMagnet, tr("Ctrl+I"));
        fileOpenMagnet->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(fileOpenMagnet, SIGNAL(triggered()), this, SLOT(slotOpenMagnet()));

        fileFileListBrowserLocal = new QAction("", this);
        fileFileListBrowserLocal->setObjectName("fileFileListBrowserLocal");
        SM->registerShortcut(fileFileListBrowserLocal, tr("Ctrl+L"));
        fileFileListBrowserLocal->setIcon(WU->getPixmap(WulforUtil::eiOWN_FILELIST));
        connect(fileFileListBrowserLocal, SIGNAL(triggered()), this, SLOT(slotFileBrowseOwnFilelist()));

        fileFileListBrowser = new QAction("", this);
        fileFileListBrowser->setObjectName("fileFileListBrowser");
        fileFileListBrowser->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(fileFileListBrowser, SIGNAL(triggered()), this, SLOT(slotFileBrowseFilelist()));

        fileFileHasher = new QAction("", this);
        fileFileHasher->setObjectName("fileFileHasher");
        fileFileHasher->setIcon(WU->getPixmap(WulforUtil::eiOPENLIST));
        connect(fileFileHasher, SIGNAL(triggered()), this, SLOT(slotFileHasher()));

        fileOpenLogFile = new QAction("", this);
        fileOpenLogFile->setObjectName("fileOpenLogFile");
        fileOpenLogFile->setIcon(WU->getPixmap(WulforUtil::eiOPEN_LOG_FILE));
        connect(fileOpenLogFile, SIGNAL(triggered()), this, SLOT(slotFileOpenLogFile()));

        fileOpenDownloadDirectory = new QAction("", this);
        fileOpenDownloadDirectory->setObjectName("fileOpenDownloadDirectory");
        fileOpenDownloadDirectory->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));
        connect(fileOpenDownloadDirectory, SIGNAL(triggered()), this, SLOT(slotFileOpenDownloadDirectory()));

        fileRefreshShareHashProgress = new QAction("", this);
        fileRefreshShareHashProgress->setObjectName("fileRefreshShareHashProgress");
        SM->registerShortcut(fileRefreshShareHashProgress, tr("Ctrl+E"));
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        connect(fileRefreshShareHashProgress, SIGNAL(triggered()), this, SLOT(slotFileRefreshShareHashProgress()));

        fileHideWindow = new QAction("", this);
        fileHideWindow->setObjectName("fileHideWindow");
        SM->registerShortcut(fileHideWindow, tr("Ctrl+Alt+H"));
        fileHideWindow->setIcon(WU->getPixmap(WulforUtil::eiHIDEWINDOW));
        connect(fileHideWindow, SIGNAL(triggered()), this, SLOT(slotHideWindow()));

        fileQuit = new QAction("", this);
        fileQuit->setObjectName("fileQuit");
        SM->registerShortcut(fileQuit, tr("Ctrl+Q"));
        fileQuit->setMenuRole(QAction::QuitRole);
        fileQuit->setIcon(WU->getPixmap(WulforUtil::eiEXIT));
        connect(fileQuit, SIGNAL(triggered()), this, SLOT(slotExit()));

        hubsHubReconnect = new QAction("", this);
        hubsHubReconnect->setObjectName("hubsHubReconnect");
        SM->registerShortcut(hubsHubReconnect, tr("Ctrl+R"));
        hubsHubReconnect->setIcon(WU->getPixmap(WulforUtil::eiRECONNECT));
        connect(hubsHubReconnect, SIGNAL(triggered()), this, SLOT(slotHubsReconnect()));

        hubsQuickConnect = new QAction("", this);
        hubsQuickConnect->setObjectName("hubsQuickConnect");
        SM->registerShortcut(hubsQuickConnect, tr("Ctrl+N"));
        hubsQuickConnect->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
        connect(hubsQuickConnect, SIGNAL(triggered()), this, SLOT(slotQC()));

        hubsFavoriteHubs = new QAction("", this);
        hubsFavoriteHubs->setObjectName("hubsFavoriteHubs");
        SM->registerShortcut(hubsFavoriteHubs, tr("Ctrl+H"));
        hubsFavoriteHubs->setIcon(WU->getPixmap(WulforUtil::eiFAVSERVER));
        connect(hubsFavoriteHubs, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteHubs()));

        hubsPublicHubs = new QAction("", this);
        hubsPublicHubs->setObjectName("hubsPublicHubs");
        SM->registerShortcut(hubsPublicHubs, tr("Ctrl+P"));
        hubsPublicHubs->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(hubsPublicHubs, SIGNAL(triggered()), this, SLOT(slotHubsPublicHubs()));

        hubsFavoriteUsers = new QAction("", this);
        hubsFavoriteUsers->setObjectName("hubsFavoriteUsers");
        SM->registerShortcut(hubsFavoriteUsers, tr("Ctrl+U"));
        hubsFavoriteUsers->setIcon(WU->getPixmap(WulforUtil::eiFAVUSERS));
        connect(hubsFavoriteUsers, SIGNAL(triggered()), this, SLOT(slotHubsFavoriteUsers()));

        toolsHubManager = new QAction("", this);
        toolsHubManager->setObjectName("toolsHubManager");
        toolsHubManager->setIcon(WU->getPixmap(WulforUtil::eiSERVER));
        connect(toolsHubManager, SIGNAL(triggered()), this, SLOT(slotToolsHubManager()));

        toolsCopyWindowTitle = new QAction("", this);
        toolsCopyWindowTitle->setObjectName("toolsCopyWindowTitle");
        toolsCopyWindowTitle->setIcon(WU->getPixmap(WulforUtil::eiEDITCOPY));
        connect(toolsCopyWindowTitle, SIGNAL(triggered()), this, SLOT(slotToolsCopyWindowTitle()));

        toolsOptions = new QAction("", this);
        toolsOptions->setObjectName("toolsOptions");
        SM->registerShortcut(toolsOptions, tr("Ctrl+O"));
        toolsOptions->setMenuRole(QAction::PreferencesRole);
        toolsOptions->setIcon(WU->getPixmap(WulforUtil::eiCONFIGURE));
        connect(toolsOptions, SIGNAL(triggered()), this, SLOT(slotToolsSettings()));

        toolsADLS = new QAction("", this);
        toolsADLS->setObjectName("toolsADLS");
        toolsADLS->setIcon(WU->getPixmap(WulforUtil::eiADLS));
        connect(toolsADLS, SIGNAL(triggered()), this, SLOT(slotToolsADLS()));

        toolsTransfers = new QAction("", this);
        toolsTransfers->setObjectName("toolsTransfers");
        SM->registerShortcut(toolsTransfers, tr("Ctrl+T"));
        toolsTransfers->setIcon(WU->getPixmap(WulforUtil::eiTRANSFER));
        toolsTransfers->setCheckable(true);
        connect(toolsTransfers, SIGNAL(toggled(bool)), this, SLOT(slotToolsTransfer(bool)));
        //transfer_dock->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

        toolsDownloadQueue = new QAction("", this);
        toolsDownloadQueue->setObjectName("toolsDownloadQueue");
        SM->registerShortcut(toolsDownloadQueue, tr("Ctrl+D"));
        toolsDownloadQueue->setIcon(WU->getPixmap(WulforUtil::eiDOWNLOAD));
        connect(toolsDownloadQueue, SIGNAL(triggered()), this, SLOT(slotToolsDownloadQueue()));

        toolsQueuedUsers = new QAction("", this);
        toolsQueuedUsers->setObjectName("toolsQueuedUsers");
        SM->registerShortcut(toolsQueuedUsers, tr("Ctrl+Shift+U"));
        toolsQueuedUsers->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(toolsQueuedUsers, SIGNAL(triggered()), this, SLOT(slotToolsQueuedUsers()));

        toolsFinishedDownloads = new QAction("", this);
        toolsFinishedDownloads->setObjectName("toolsFinishedDownloads");
        toolsFinishedDownloads->setIcon(WU->getPixmap(WulforUtil::eiDOWNLIST));
        connect(toolsFinishedDownloads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedDownloads()));

        toolsFinishedUploads = new QAction("", this);
        toolsFinishedUploads->setObjectName("toolsFinishedUploads");
        toolsFinishedUploads->setIcon(WU->getPixmap(WulforUtil::eiUPLIST));
        connect(toolsFinishedUploads, SIGNAL(triggered()), this, SLOT(slotToolsFinishedUploads()));

        toolsSpy = new QAction("", this);
        toolsSpy->setObjectName("toolsSpy");
        toolsSpy->setIcon(WU->getPixmap(WulforUtil::eiSPY));
        connect(toolsSpy, SIGNAL(triggered()), this, SLOT(slotToolsSpy()));

        toolsAntiSpam = new QAction("", this);
        toolsAntiSpam->setObjectName("toolsAntiSpam");
        toolsAntiSpam->setIcon(WU->getPixmap(WulforUtil::eiSPAM));
        toolsAntiSpam->setCheckable(true);
        toolsAntiSpam->setChecked(AntiSpam::getInstance() != NULL);
        connect(toolsAntiSpam, SIGNAL(triggered()), this, SLOT(slotToolsAntiSpam()));

        toolsIPFilter = new QAction("", this);
        toolsIPFilter->setObjectName("toolsIPFilter");
        toolsIPFilter->setIcon(WU->getPixmap(WulforUtil::eiFILTER));
        toolsIPFilter->setCheckable(true);
        toolsIPFilter->setChecked(IPFilter::getInstance() != NULL);
        connect(toolsIPFilter, SIGNAL(triggered()), this, SLOT(slotToolsIPFilter()));

        toolsAwayOn = new QAction("", this);
        toolsAwayOn->setObjectName("toolsAwayOn");
        toolsAwayOn->setCheckable(true);
        connect(toolsAwayOn, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        toolsAwayOff = new QAction("", this);
        toolsAwayOff->setObjectName("toolsAwayOff");
        toolsAwayOff->setCheckable(true);
        connect(toolsAwayOff, SIGNAL(triggered()), this, SLOT(slotToolsSwitchAway()));

        toolsAutoAway = new QAction("", this);
        toolsAutoAway->setCheckable(true);
        toolsAutoAway->setChecked(WBGET(WB_APP_AUTO_AWAY));
        connect(toolsAutoAway, SIGNAL(triggered()), this, SLOT(slotToolsAutoAway()));

#ifdef USE_JS
        toolsJS = new QAction("", this);
        toolsJS->setObjectName("toolsJS");
        toolsJS->setIcon(WU->getPixmap(WulforUtil::eiPLUGIN));
        connect(toolsJS, SIGNAL(triggered()), this, SLOT(slotToolsJS()));

        toolsJSConsole = new QAction("", this);
        toolsJSConsole->setObjectName("toolsJSConsole");
        toolsJSConsole->setIcon(WU->getPixmap(WulforUtil::eiCONSOLE));
        connect(toolsJSConsole, SIGNAL(triggered()), this, SLOT(slotToolsJSConsole()));
#endif

        menuAwayAction = new QAction("", this);
        // submenu
        QAction *away_sep = new QAction("", this);
        away_sep->setSeparator(true);

        awayGroup = new QActionGroup(this);
        awayGroup->addAction(toolsAwayOn);
        awayGroup->addAction(toolsAwayOff);

        menuAway = new QMenu(this);
        menuAway->addActions(QList<QAction*>() << toolsAwayOn << toolsAwayOff << away_sep << toolsAutoAway);
        {
            QAction *act = Util::getAway()? toolsAwayOn : toolsAwayOff;
            act->setChecked(true);
        }
        // end
        menuAwayAction->setMenu(menuAway);
        menuAwayAction->setIcon(QIcon(WU->getPixmap(WulforUtil::eiAWAY)));

        toolsSearch = new QAction("", this);
        toolsSearch->setObjectName("toolsSearch");
        SM->registerShortcut(toolsSearch, tr("Ctrl+S"));
        toolsSearch->setIcon(WU->getPixmap(WulforUtil::eiFILEFIND));
        connect(toolsSearch, SIGNAL(triggered()), this, SLOT(slotToolsSearch()));

        toolsHideProgressSpace = new QAction("", this);
        toolsHideProgressSpace->setObjectName("toolsHideProgressSpace");

#if (!defined FREE_SPACE_BAR_C)
        toolsHideProgressSpace->setVisible(false);
#endif
        toolsHideProgressSpace->setIcon(WU->getPixmap(WulforUtil::eiFREESPACE));
        connect(toolsHideProgressSpace, SIGNAL(triggered()), this, SLOT(slotHideProgressSpace()));

        toolsHideLastStatus = new QAction("", this);
        toolsHideLastStatus->setObjectName("toolsHideLastStatus");
        toolsHideLastStatus->setIcon(WU->getPixmap(WulforUtil::eiSTATUS));
        connect(toolsHideLastStatus, SIGNAL(triggered()), this, SLOT(slotHideLastStatus()));

        toolsHideUsersStatisctics = new QAction("", this);
        toolsHideUsersStatisctics->setObjectName("toolsHideUsersStatisctics");
        toolsHideUsersStatisctics->setIcon(WU->getPixmap(WulforUtil::eiUSERS));
        connect(toolsHideUsersStatisctics, SIGNAL(triggered()), this, SLOT(slotHideUsersStatistics()));

        toolsSwitchSpeedLimit = new QAction("", this);
        toolsSwitchSpeedLimit->setObjectName("toolsSwitchSpeedLimit");
        toolsSwitchSpeedLimit->setIcon(BOOLSETTING(THROTTLE_ENABLE)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
        toolsSwitchSpeedLimit->setCheckable(true);
        toolsSwitchSpeedLimit->setChecked(BOOLSETTING(THROTTLE_ENABLE));
        connect(toolsSwitchSpeedLimit, SIGNAL(triggered()), this, SLOT(slotToolsSwitchSpeedLimit()));

        chatClear = new QAction("", this);
        chatClear->setObjectName("chatClear");
        chatClear->setIcon(WU->getPixmap(WulforUtil::eiCLEAR));
        connect(chatClear, SIGNAL(triggered()), this, SLOT(slotChatClear()));

        findInWidget = new QAction("", this);
        findInWidget->setObjectName("findInWidget");
        SM->registerShortcut(findInWidget, tr("Ctrl+F"));
        findInWidget->setIcon(WU->getPixmap(WulforUtil::eiFIND));
        connect(findInWidget, SIGNAL(triggered()), this, SLOT(slotFind()));

        chatDisable = new QAction("", this);
        chatDisable->setObjectName("chatDisable");
        chatDisable->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
        connect(chatDisable, SIGNAL(triggered()), this, SLOT(slotChatDisable()));

        QAction *separator0 = new QAction("", this);
        separator0->setObjectName("separator0");
        separator0->setSeparator(true);
        QAction *separator1 = new QAction("", this);
        separator1->setObjectName("separator1");
        separator1->setSeparator(true);
        QAction *separator2 = new QAction("", this);
        separator2->setObjectName("separator2");
        separator2->setSeparator(true);
        QAction *separator3 = new QAction("", this);
        separator3->setObjectName("separator3");
        separator3->setSeparator(true);
        QAction *separator4 = new QAction("", this);
        separator4->setObjectName("separator4");
        separator4->setSeparator(true);
        QAction *separator5 = new QAction("", this);
        separator5->setObjectName("separator5");
        separator5->setSeparator(true);
        QAction *separator6 = new QAction("", this);
        separator6->setObjectName("separator6");
        separator6->setSeparator(true);

        /*fileMenuActions << fileOpenMagnet
                << separator3
                << fileFileListBrowser
                << fileFileListBrowserLocal
                << fileRefreshShareHashProgress
                << separator0
                << fileOpenLogFile
                << fileOpenDownloadDirectory
                << fileFileHasher
                << separator1
                << fileHideWindow
                << separator2
                << fileQuit;

        hubsMenuActions << hubsHubReconnect
                << hubsQuickConnect
                << hubsFavoriteHubs
                << hubsPublicHubs
                << separator0
                << hubsFavoriteUsers;

        toolsMenuActions << toolsSearch
                << toolsADLS
                << separator0
                << toolsTransfers
                << toolsDownloadQueue
                << toolsQueuedUsers
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << toolsSwitchSpeedLimit
                //<< toolsHubManager
                << separator1
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter
                << separator2
                << menuAwayAction
                << separator3
                << toolsHideProgressSpace
                << toolsHideLastStatus
                << toolsHideUsersStatisctics
#ifdef USE_JS
                << separator6
                << toolsJS
                << toolsJSConsole
#endif
                << separator4
                << toolsCopyWindowTitle
                << separator5
                << toolsOptions;*/

        toolBarActions << toolsOptions
                << separator0
                << fileFileListBrowserLocal
                << fileRefreshShareHashProgress
                << separator1
                << hubsHubReconnect
                << hubsQuickConnect
                << separator2
                << hubsFavoriteHubs
                << hubsFavoriteUsers
                << toolsSearch
                << hubsPublicHubs
                << separator3
                << toolsTransfers
                << toolsDownloadQueue
                << toolsFinishedDownloads
                << toolsFinishedUploads
                << toolsSwitchSpeedLimit
                << separator4
                << chatClear
                << findInWidget
                << chatDisable
                << separator5
                << toolsADLS
                << toolsSpy
                << toolsAntiSpam
                << toolsIPFilter;
                //<< separator6
                //<< fileQuit;
    }
    {
        menuWidgets = new QMenu("", this);
    }
    {
        // submenu
        nextTabShortCut     = new  QAction(this);
        prevTabShortCut     = new  QAction(this);
        nextMsgShortCut     = new  QAction(this);
        prevMsgShortCut     = new  QAction(this);
        closeWidgetShortCut      = new  QAction(this);
        toggleMainMenuShortCut   = new  QAction(this);

        nextTabShortCut->setObjectName("nextTabShortCut");
        prevTabShortCut->setObjectName("prevTabShortCut");
        nextMsgShortCut->setObjectName("nextMsgShortCut");
        prevMsgShortCut->setObjectName("prevMsgShortCut");
        closeWidgetShortCut->setObjectName("closeWidgetShortCut");
        toggleMainMenuShortCut->setObjectName("toggleMainMenuShortCut");

        nextTabShortCut->setText(tr("Next widget"));
        prevTabShortCut->setText(tr("Previous widget"));
        nextMsgShortCut->setText(tr("Next message"));
        prevMsgShortCut->setText(tr("Previous message"));
        closeWidgetShortCut->setText(tr("Close current widget"));
        toggleMainMenuShortCut->setText(tr("Toggle main menu"));

        nextTabShortCut->setShortcutContext(Qt::ApplicationShortcut);
        prevTabShortCut->setShortcutContext(Qt::ApplicationShortcut);
        nextMsgShortCut->setShortcutContext(Qt::ApplicationShortcut);
        prevMsgShortCut->setShortcutContext(Qt::ApplicationShortcut);
        closeWidgetShortCut->setShortcutContext(Qt::ApplicationShortcut);
        toggleMainMenuShortCut->setShortcutContext(Qt::ApplicationShortcut);

        SM->registerShortcut(nextTabShortCut, tr("Ctrl+PgDown"));
        SM->registerShortcut(prevTabShortCut, tr("Ctrl+PgUp"));
        SM->registerShortcut(nextMsgShortCut, tr("Ctrl+Down"));
        SM->registerShortcut(prevMsgShortCut, tr("Ctrl+Up"));
        SM->registerShortcut(closeWidgetShortCut, tr("Ctrl+W"));
        SM->registerShortcut(toggleMainMenuShortCut, tr("Ctrl+M"));

        connect(nextMsgShortCut,        SIGNAL(triggered()), this, SLOT(nextMsg()));
        connect(prevMsgShortCut,        SIGNAL(triggered()), this, SLOT(prevMsg()));
        connect(closeWidgetShortCut,    SIGNAL(triggered()), this, SLOT(slotCloseCurrentWidget()));
        connect(toggleMainMenuShortCut, SIGNAL(triggered()), this, SLOT(slotHideMainMenu()));
        // end

        sh_menu = new QMenu(this);
        sh_menu->addActions(QList<QAction*>()
                            << nextTabShortCut
                            << prevTabShortCut
                            << nextMsgShortCut
                            << prevMsgShortCut
                            << closeWidgetShortCut
                            << toggleMainMenuShortCut);

        panelsWidgets = new QAction("", this);
        panelsWidgets->setCheckable(true);
        connect(panelsWidgets, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        panelsTools = new QAction("", this);
        panelsTools->setCheckable(true);
        connect(panelsTools, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));

        panelsSearch = new QAction("", this);
        panelsSearch->setCheckable(true);
        connect(panelsSearch, SIGNAL(triggered()), this, SLOT(slotPanelMenuActionClicked()));
    }
    {
        aboutHomepage = new QAction("", this);
        aboutHomepage->setMenuRole(QAction::AboutRole);
        connect(aboutHomepage, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        aboutSource = new QAction("", this);
        aboutSource->setMenuRole(QAction::AboutRole);
        connect(aboutSource, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        aboutIssues = new QAction("", this);
        aboutIssues->setMenuRole(QAction::AboutRole);
        connect(aboutIssues, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        aboutWiki = new QAction("", this);
        aboutWiki->setMenuRole(QAction::AboutRole);
        connect(aboutWiki, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        aboutChangelog = new QAction("", this);
        aboutChangelog->setMenuRole(QAction::AboutRole);
        connect(aboutChangelog, SIGNAL(triggered()), this, SLOT(slotAboutOpenUrl()));

        aboutClient = new QAction("", this);
        aboutClient->setMenuRole(QAction::AboutRole);
        aboutClient->setIcon(WU->getPixmap(WulforUtil::eiICON_APPL)
                    .scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        connect(aboutClient, SIGNAL(triggered()), this, SLOT(slotAboutClient()));

        aboutQt = new QAction("", this);
        aboutQt->setMenuRole(QAction::AboutQtRole);
        aboutQt->setIcon(WU->getPixmap(WulforUtil::eiQT_LOGO));
        connect(aboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    }
}

void MainLayout::initMenuBar(){
#ifdef Q_WS_MAC
    setMenuBar(new QMenuBar());
    menuBar()->setParent(NULL);
    connect(this, SIGNAL(destroyed()), menuBar(), SLOT(deleteLater()));
#endif

    {
        menuFile = new QMenu("", this);

        menuFile->addActions(fileMenuActions);
    }
    {
        menuHubs = new QMenu("", this);

        menuHubs->addActions(hubsMenuActions);
    }
    {
        menuTools = new QMenu("", this);

        menuTools->addActions(toolsMenuActions);
    }
    {
        QAction *sep0 = new QAction("", this);
        sep0->setSeparator(true);

        menuPanels = new QMenu("", this);

        menuPanels->addMenu(sh_menu);
        menuPanels->addAction(sep0);
        menuPanels->addAction(panelsWidgets);
        menuPanels->addAction(panelsTools);
        menuPanels->addAction(panelsSearch);
    }
    {
        QAction *sep0 = new QAction("", this);
        sep0->setSeparator(true);
        QAction *sep1 = new QAction("", this);
        sep1->setSeparator(true);

        menuAbout = new QMenu("", this);

        menuAbout->addAction(aboutHomepage);
        menuAbout->addAction(aboutSource);
        menuAbout->addAction(aboutIssues);
        menuAbout->addAction(aboutWiki);
        menuAbout->addAction(sep0);
        menuAbout->addAction(aboutChangelog);
        menuAbout->addAction(sep1);
        menuAbout->addAction(aboutClient);
        menuAbout->addAction(aboutQt);
    }

    menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuHubs);
    menuBar()->addMenu(menuTools);
    menuBar()->addMenu(menuWidgets);
    menuBar()->addMenu(menuPanels);
    menuBar()->addMenu(menuAbout);
    menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainLayout::initStatusBar(){
    statusLabel = new QLabel(statusBar());
    statusLabel->setFrameShadow(QFrame::Sunken);
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusLabel->setToolTip(tr("Counts"));
    statusLabel->setContentsMargins(0, 0, 0, 0);

    statusSPLabel = new QLabel(statusBar());
    statusSPLabel->setFrameShadow(QFrame::Sunken);
    statusSPLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusSPLabel->setToolTip(tr("Download/Upload speed"));
    statusSPLabel->setContentsMargins(0, 0, 0, 0);

    statusDLabel = new QLabel(statusBar());
    statusDLabel->setFrameShadow(QFrame::Sunken);
    statusDLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusDLabel->setToolTip(tr("Downloaded/Uploaded"));
    statusDLabel->setContentsMargins(0, 0, 0, 0);

    msgLabel = new QLabel(statusBar());
    msgLabel->setFrameShadow(QFrame::Plain);
    msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    msgLabel->setWordWrap(true);
    msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    msgLabel->setContentsMargins(0, 0, 0, 0);

#if (defined FREE_SPACE_BAR_C)
    progressSpace = new QProgressBar(this);
    progressSpace->setMaximum(100);
    progressSpace->setMinimum(0);
    progressSpace->setAlignment( Qt::AlignHCenter );
    progressSpace->setMinimumWidth(100);
    progressSpace->setMaximumWidth(250);
    progressSpace->setFixedHeight(18);
    progressSpace->setToolTip(tr("Space free"));
    progressSpace->installEventFilter(this);

    if (!WBGET(WB_SHOW_FREE_SPACE))
        progressSpace->hide();
#else //FREE_SPACE_BAR_C
    WBSET(WB_SHOW_FREE_SPACE, false);
#endif //FREE_SPACE_BAR_C

    progressHashing = new QProgressBar(this);
    progressHashing->setMaximum(100);
    progressHashing->setMinimum(0);
    progressHashing->setAlignment( Qt::AlignHCenter );
    progressHashing->setFixedHeight(18);
    progressHashing->setToolTip(tr("Hashing progress"));
    progressHashing->hide();
    progressHashing->installEventFilter( this );

    statusBar()->addWidget(progressHashing);
    statusBar()->addWidget(msgLabel, 1);
    statusBar()->addPermanentWidget(statusDLabel);
    statusBar()->addPermanentWidget(statusSPLabel);
    statusBar()->addPermanentWidget(statusLabel);
#if (defined FREE_SPACE_BAR_C)
    statusBar()->addPermanentWidget(progressSpace);
#endif //FREE_SPACE_BAR_C
}

void MainLayout::initSearchBar(){
    searchLineEdit = new LineEdit(this);

    connect(searchLineEdit,   SIGNAL(returnPressed()), this, SLOT(slotToolsSearch()));
}

void MainLayout::retranslateUi(){
    //Retranslate menu actions
    {
        menuFile->setTitle(tr("&File"));

        fileOpenMagnet->setText(tr("Open magnet link"));

        fileOpenLogFile->setText(tr("Open log file"));

        fileOpenDownloadDirectory->setText(tr("Open download directory"));

        fileFileListBrowser->setText(tr("Open filelist..."));

        fileFileHasher->setText(tr("Calculate file TTH"));

        fileFileListBrowserLocal->setText(tr("Open own filelist"));

        fileRefreshShareHashProgress->setText(tr("Refresh share"));

        fileHideWindow->setText(tr("Hide window"));

        if (!WBGET(WB_TRAY_ENABLED))
            fileHideWindow->setText(tr("Show/hide find frame"));

        fileQuit->setText(tr("Quit"));

        menuHubs->setTitle(tr("&Hubs"));

        hubsHubReconnect->setText(tr("Reconnect to hub"));

        hubsFavoriteHubs->setText(tr("Favourite hubs"));

        hubsPublicHubs->setText(tr("Public hubs"));

        hubsFavoriteUsers->setText(tr("Favourite users"));

        hubsQuickConnect->setText(tr("Quick connect"));

        menuTools->setTitle(tr("&Tools"));

        toolsTransfers->setText(tr("Transfers"));

        toolsDownloadQueue->setText(tr("Download queue"));

        toolsQueuedUsers->setText(tr("Queued Users"));

        toolsHubManager->setText(tr("Hub Manager"));

        toolsFinishedDownloads->setText(tr("Finished downloads"));

        toolsFinishedUploads->setText(tr("Finished uploads"));

        toolsSpy->setText(tr("Search Spy"));

        toolsAntiSpam->setText(tr("AntiSpam module"));

        toolsIPFilter->setText(tr("IPFilter module"));

        toolsHideProgressSpace->setText(tr("Hide free space bar"));

        if (!WBGET(WB_SHOW_FREE_SPACE))
            toolsHideProgressSpace->setText(tr("Show free space bar"));

        toolsHideLastStatus->setText(tr("Hide last status message"));

        if (!WBGET(WB_LAST_STATUS))
            toolsHideLastStatus->setText(tr("Show last status message"));

        toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

        if (!WBGET(WB_USERS_STATISTICS))
            toolsHideUsersStatisctics->setText(tr("Show users statistics"));

        menuAway->setTitle(tr("Away message"));

        toolsAwayOn->setText(tr("On"));

        toolsAwayOff->setText(tr("Off"));

        toolsAutoAway->setText(tr("Away when not visible"));

        toolsCopyWindowTitle->setText(tr("Copy window title"));

        toolsOptions->setText(tr("Preferences"));

        toolsSearch->setText(tr("Search"));

        toolsADLS->setText(tr("ADLSearch"));

        toolsSwitchSpeedLimit->setText(tr("Speed limit On/Off"));

#ifdef USE_JS
        toolsJS->setText(tr("Scripts Manager"));

        toolsJSConsole->setText(tr("Script Console"));
#endif

        chatClear->setText(tr("Clear chat"));

        findInWidget->setText(tr("Find/Filter"));

        chatDisable->setText(tr("Disable/enable chat"));

        menuWidgets->setTitle(tr("&Widgets"));

        menuPanels->setTitle(tr("&Panels"));

        if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
            panelsWidgets->setText(tr("Widgets panel"));
        else
            panelsWidgets->setText(tr("Widgets side dock"));

        panelsTools->setText(tr("Tools panel"));

        panelsSearch->setText(tr("Fast search panel"));

        menuAbout->setTitle(tr("&Help"));

        aboutHomepage->setText(tr("Homepage"));

        aboutSource->setText(tr("Source (git)"));

        aboutIssues->setText(tr("Report a Bug"));

        aboutWiki->setText(tr("Wiki of project"));

        aboutChangelog->setText(tr("Changelog (git)"));

        aboutClient->setText(tr("About EiskaltDC++"));

        aboutQt->setText(tr("About Qt"));
    }
    {
        sh_menu->setTitle(tr("Actions"));
    }
    {
        arena->setWindowTitle(tr("Main layout"));
    }
}

void MainLayout::initToolbar(){
    fBar = new ToolBar(this);
    fBar->setObjectName("fBar");

    QStringList enabled_actions = QString(QByteArray::fromBase64(WSGET(WS_MAINWINDOW_TOOLBAR_ACTS).toAscii())).split(";", QString::SkipEmptyParts);

    if (enabled_actions.isEmpty())
        fBar->addActions(toolBarActions);
    else {
        foreach (QString objName, enabled_actions){
            QAction *act = findChild<QAction*>(objName);

            if (act)
                fBar->addAction(act);
        }
    }

    fBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fBar->setMovable(true);
    fBar->setFloatable(true);
    fBar->setAllowedAreas(Qt::AllToolBarAreas);
    fBar->setWindowTitle(tr("Actions"));
    fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(WIGET(TOOLBUTTON_STYLE, Qt::ToolButtonIconOnly)));

    connect(fBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotToolbarCustomization()));

    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR) && WBGET(WB_MAINWINDOW_USE_M_TABBAR)){

        mBar = new MultiLineToolBar(this);
        mBar->setContextMenuPolicy(Qt::CustomContextMenu);

        addToolBar(mBar);

        wcontainer = static_cast<ArenaWidgetContainer*>(mBar);
    }
    else if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR) && !WBGET(WB_MAINWINDOW_USE_M_TABBAR)){

        tBar = new ToolBar(this);
        tBar->setObjectName("tBar");
        tBar->initTabs();
        tBar->setMovable(true);
        tBar->setFloatable(true);
        tBar->setAllowedAreas(Qt::AllToolBarAreas);
        tBar->setContextMenuPolicy(Qt::CustomContextMenu);

        addToolBar(tBar);

        wcontainer = static_cast<ArenaWidgetContainer*>(tBar);
    }

    if (tBar){
        connect(nextTabShortCut, SIGNAL(triggered()), tBar, SLOT(nextTab()));
        connect(prevTabShortCut, SIGNAL(triggered()), tBar, SLOT(prevTab()));
    }
    else if (mBar){
        connect(nextTabShortCut, SIGNAL(triggered()), mBar, SIGNAL(nextTab()));
        connect(prevTabShortCut, SIGNAL(triggered()), mBar, SIGNAL(prevTab()));
    }

    sBar = new ToolBar(this);
    sBar->setObjectName("sBar");
    sBar->addWidget(searchLineEdit);
    sBar->setContextMenuPolicy(Qt::CustomContextMenu);
    sBar->setMovable(true);
    sBar->setFloatable(true);
    sBar->setAllowedAreas(Qt::AllToolBarAreas);
    addToolBar(sBar);
}

void MainLayout::initSideBar(){
    if (!WBGET(WB_MAINWINDOW_USE_SIDEBAR))
        return;

    SideBarModel *model = new SideBarModel(this);
    sideTree = new QTreeView(this);
    sideDock = new QDockWidget("", this);

    sideDock->setWidget(sideTree);
    sideDock->setFeatures(sideDock->features() & (~QDockWidget::DockWidgetClosable));
    sideDock->setObjectName("sideDock");
    sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    sideDock->setContextMenuPolicy(Qt::CustomContextMenu);
    sideTree->setModel(model);
    sideTree->setItemsExpandable(true);
    sideTree->setHeaderHidden(true);
    sideTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sideTree->setContextMenuPolicy(Qt::CustomContextMenu);
    sideTree->setItemDelegate(new SideBarDelegate(sideTree));
    sideTree->expandAll();

    sideTree->installEventFilter(this);

    wcontainer = static_cast<ArenaWidgetContainer*>(model);

    _lc_MW->addDockWidget(Qt::LeftDockWidgetArea, sideDock);

    connect(sideDock, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSideBarDockMenu()));
    connect(sideTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotSideBarDblClicked(QModelIndex)));
    connect(sideTree, SIGNAL(clicked(QModelIndex)),     this, SLOT(slotSidebarHook(QModelIndex)));
    connect(sideTree, SIGNAL(clicked(QModelIndex)),    model, SLOT(slotIndexClicked(QModelIndex)));
    connect(sideTree, SIGNAL(activated(QModelIndex)),  model, SLOT(slotIndexClicked(QModelIndex)));
    connect(sideTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSidebarContextMenu()));
    connect(model,    SIGNAL(mapWidget(ArenaWidget*)), this,  SLOT(mapWidgetOnArena(ArenaWidget*)));
    connect(model,    SIGNAL(selectIndex(QModelIndex)),this,  SLOT(slotSelectSidebarIndex(QModelIndex)));
}

QObject *MainLayout::getToolBar(){
    if (!fBar)
        return NULL;

    return qobject_cast<QObject*>(reinterpret_cast<QToolBar*>(fBar->qt_metacast("QToolBar")));
}

ArenaWidget *MainLayout::widgetForRole(ArenaWidget::Role r) const{
    ArenaWidget *awgt = NULL;

    switch (r){
    case ArenaWidget::Downloads:
        {
            if (!DownloadQueue::getInstance()) DownloadQueue::newInstance();
            awgt = DownloadQueue::getInstance();

            break;
        }
    case ArenaWidget::FinishedUploads:
        {
            if (!FinishedUploads::getInstance()) FinishedUploads::newInstance();
            awgt = FinishedUploads::getInstance();

            break;
        }
    case ArenaWidget::FinishedDownloads:
        {
            if (!FinishedDownloads::getInstance()) FinishedDownloads::newInstance();
            awgt = FinishedDownloads::getInstance();

            break;
        }
    case ArenaWidget::FavoriteHubs:
        {
            if (!FavoriteHubs::getInstance()) FavoriteHubs::newInstance();
            awgt = FavoriteHubs::getInstance();

            break;
        }
    case ArenaWidget::FavoriteUsers:
        {
            if (!FavoriteUsers::getInstance()) FavoriteUsers::newInstance();
            awgt = FavoriteUsers::getInstance();

            break;
        }
    case ArenaWidget::PublicHubs:
        {
            if (!PublicHubs::getInstance()) PublicHubs::newInstance();
            awgt = PublicHubs::getInstance();

            break;
        }
    case ArenaWidget::Spy:
        {
            if (!SpyFrame::getInstance()) SpyFrame::newInstance();
            awgt = SpyFrame::getInstance();

            break;
        }
    case ArenaWidget::ADLS:
        {
            if (!ADLS::getInstance()) ADLS::newInstance();
            awgt = ADLS::getInstance();

            break;
        }
    case ArenaWidget::QueuedUsers:
        {
            awgt = QueuedUsers::getInstance();// QueuedUsers::newInstance() called at startup

            break;
        }
    default:
        break;
    }

    return awgt;
}

void MainLayout::newHubFrame(QString address, QString enc){
    if (address.isEmpty())
        return;

    HubFrame *fr = NULL;

    if (fr = HubManager::getInstance()->getHub(address)){
        mapWidgetOnArena(fr);

        return;
    }

    fr = new HubFrame(NULL, address, enc);
    fr->setAttribute(Qt::WA_DeleteOnClose);

    addArenaWidget(fr);
    addArenaWidgetOnToolbar(fr);

    mapWidgetOnArena(fr);
}

void MainLayout::updateStatus(const QMap<QString, QString> &map){
    if (!statusLabel)
        return;

    QString statsText = map["STATS"];
    QFontMetrics metrics(statusLabel->font());

    statusLabel->setText(statsText);

    QString speedText = tr("%1/s / %2/s").arg(map["DSPEED"]).arg(map["USPEED"]);
    QString downText = tr("%1 / %2").arg(map["DOWN"]).arg(map["UP"]);

    statusSPLabel->setText(speedText);
    statusDLabel->setText(downText);

    if (Notification::getInstance())
        Notification::getInstance()->setToolTip(map["DSPEED"]+tr("/s"), map["USPEED"]+tr("/s"), map["DOWN"], map["UP"]);

    int leftM=0, topM=0, rightM=0, bottomM=0;
    statusSPLabel->getContentsMargins(&leftM, &topM, &rightM, &bottomM);
    int boundWidth = leftM + rightM;

    statusSPLabel->setFixedWidth(metrics.width(speedText) > statusSPLabel->width()? metrics.width(speedText) + boundWidth : statusSPLabel->width());
    statusDLabel->setFixedWidth(metrics.width(downText) > statusDLabel->width()? metrics.width(downText) + boundWidth : statusDLabel->width());

    if (WBGET(WB_SHOW_FREE_SPACE)) {
#ifdef FREE_SPACE_BAR_C
        std::string s = SETTING(DOWNLOAD_DIRECTORY);
        unsigned long long available = 0;
        unsigned long long total = 0;
        if (!s.empty()) {
            if (FreeSpace::FreeDiscSpace(s, &available, &total) == false) {
                available = 0;
                total = 0;
            }
        }
        float percent = 100.0f*(total-available)/total;
        QString format = tr("Free %1")
                         .arg(WulforUtil::formatBytes(available));

        QString tooltip = tr("Free %1 of %2")
                          .arg(WulforUtil::formatBytes(available))
                          .arg(WulforUtil::formatBytes(total));

        progressSpace->setFormat(format);
        progressSpace->setToolTip(tooltip);
        progressSpace->setValue(static_cast<unsigned>(percent));

        progressSpace->setFixedWidth(metrics.width(format) > progressSpace->width()? metrics.width(progressSpace->text()) + 40 : progressSpace->width());
#endif //FREE_SPACE_BAR_C
    }

    if ((Util::getAway() && !toolsAwayOn->isChecked()) || (!Util::getAway() && toolsAwayOff->isChecked())){
        QAction *act = Util::getAway()? toolsAwayOn : toolsAwayOff;

        act->setChecked(true);
    }

    updateHashProgressStatus();
}

void MainLayout::updateHashProgressStatus() {
    WulforUtil *WU = WulforUtil::getInstance();

    switch( HashProgress::getHashStatus() ) {
    case HashProgress::IDLE:
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiREFRLIST));
        fileRefreshShareHashProgress->setText(tr("Refresh share"));
        {
            progress_dialog()->resetProgress(); // Here dialog will be actually created
            progressHashing->hide();
        }
        //qDebug("idle");
        break;
    case HashProgress::LISTUPDATE:
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            progressHashing->setValue( 100 );
            progressHashing->setFormat(tr("List update"));
            progressHashing->show();
        }
        //qDebug("listupdate");
        break;
    case HashProgress::DELAYED:
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            if (SETTING(HASHING_START_DELAY) >= 0){
                int left = SETTING(HASHING_START_DELAY) - Util::getUpTime();
                progressHashing->setValue( 100 * left / SETTING(HASHING_START_DELAY) );
                progressHashing->setFormat(tr("Delayed"));
                progressHashing->show();
            }
            else {
                progressHashing->hide();
            }
        }
        //qDebug("delayed");
        break;
    case HashProgress::PAUSED:
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            if (SETTING(HASHING_START_DELAY) >= 0){
                progressHashing->setValue( 100 );
                progressHashing->setFormat(tr("Paused"));
                progressHashing->show();
            }
            else {
                progressHashing->hide();
            }
        }
        //qDebug("paused");
        break;
    case HashProgress::RUNNING:
        fileRefreshShareHashProgress->setIcon(WU->getPixmap(WulforUtil::eiHASHING));
        fileRefreshShareHashProgress->setText(tr("Hash progress"));
        {
            int progress = static_cast<int>( progress_dialog()->getProgress()*100 );
            progressHashing->setFormat(tr("%p%"));
            progressHashing->setValue( progress );
            progressHashing->show();
        }
        //qDebug("running");
        break;
    default:
        //qDebug("unknown");
        break;
    }
}

void MainLayout::setStatusMessage(QString msg){
    QFontMetrics m(msgLabel->font());
    QString pure_msg = msg;

    if (m.width(msg) > msgLabel->width())
        pure_msg = m.elidedText(msg, Qt::ElideRight, msgLabel->width(), 0);

    WulforUtil::getInstance()->textToHtml(pure_msg, true);
    WulforUtil::getInstance()->textToHtml(msg, true);

    msgLabel->setText(pure_msg);

    core_msg_history.push_back(msg);

    if (WIGET(WI_STATUSBAR_HISTORY_SZ) > 0){
        while (core_msg_history.size() > WIGET(WI_STATUSBAR_HISTORY_SZ))
            core_msg_history.removeFirst();
    }
    else
        core_msg_history.clear();

    msgLabel->setToolTip(core_msg_history.join("\n"));
    msgLabel->setMaximumHeight(statusLabel->height());
}

void MainLayout::autoconnect(){
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        if (entry->getConnect()) {
            if (entry->getNick().empty() && SETTING(NICK).empty())
                continue;

            QString encoding = WulforUtil::getInstance()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

            newHubFrame(QString::fromStdString(entry->getServer()), encoding);
        }
    }
}

void MainLayout::parseCmdLine(){
    QStringList args = qApp->arguments();

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?")){
            Magnet m(this);
            m.setLink(arg);
            if (WIGET(WI_DEF_MAGNET_ACTION) == 0) {
                m.exec();
            }
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainLayout::parseInstanceLine(QString data){
    if (!isVisible()){
        show();
        raise();

        redrawToolPanel();
    }

    QStringList args = data.split("\n", QString::SkipEmptyParts);

    foreach (QString arg, args){
        if (arg.startsWith("magnet:?xt=urn:tree:tiger:")){
            Magnet m(this);
            m.setLink(arg);
            if (WIGET(WI_DEF_MAGNET_ACTION) == 0) {
                m.exec();
            }
        }
        else if (arg.startsWith("dchub://")){
            newHubFrame(arg, "");
        }
        else if (arg.startsWith("adc://") || arg.startsWith("adcs://")){
            newHubFrame(arg, "UTF-8");
        }
    }
}

void MainLayout::browseOwnFiles(){
    slotFileBrowseOwnFilelist();
}

void MainLayout::slotFileBrowseFilelist(){
    QString file = QFileDialog::getOpenFileName(this, tr("Choose file to open"),
                QString::fromStdString(Util::getPath(Util::PATH_FILE_LISTS)),
                tr("Modern XML Filelists") + " (*.xml.bz2);;" +
                tr("Modern XML Filelists uncompressed") + " (*.xml);;" +
                tr("All files") + " (*)");

    if (file.isEmpty())
        return;

    file = QDir::toNativeSeparators(file);
    UserPtr user = DirectoryListing::getUserFromFilename(_tq(file));

    if (user) {
        new ShareBrowser(user, file, "");
    } else {
        setStatusMessage(tr("Unable to load file list: Invalid file list name"));
    }
}

void MainLayout::redrawToolPanel(){
    wcontainer->redraw();

    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
    QHash<QAction*, ArenaWidget*>::iterator end = menuWidgetsHash.end();

    PMWindow *pm = NULL;
    bool has_unread = false;

    for(; it != end; ++it){//also redraw all widget menu items
        it.key()->setText(it.value()->getArenaShortTitle());
        it.key()->setIcon(it.value()->getPixmap());

        emit changeTabName(it.value()->getWidget(), it.value()->getArenaShortTitle());
        emit changeTabIcon(it.value()->getWidget(), it.value()->getPixmap());

        pm = qobject_cast<PMWindow *>(arenaMap[it.value()]);

        if (pm && pm->hasNewMessages())
            has_unread = true;
    }

    if (!has_unread)
        Notify->resetTrayIcon();
}

void MainLayout::addArenaWidget(ArenaWidget *wgt){
    if (!arenaWidgets.contains(wgt) && wgt && wgt->getWidget()){
        arenaWidgets.push_back(wgt);
        arenaMap[wgt] = wgt->getWidget();
    }
}

void MainLayout::remArenaWidget(ArenaWidget *awgt){
    if (arenaWidgets.contains(awgt)){
        arenaWidgets.removeAt(arenaWidgets.indexOf(awgt));
        arenaMap.erase(arenaMap.find(awgt));

        if (arena->widget() == awgt->getWidget()){
            chatClear->setEnabled(false);
            findInWidget->setEnabled(false);
            chatDisable->setEnabled(false);
        }
    }
}

void MainLayout::mapWidgetOnArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (arena->widget() == awgt->getWidget()){
        wcontainer->mapped(awgt);

        awgt->requestFocus();

        return;
    }

    emit raiseTab(awgt->getWidget());

    setWindowTitle(awgt->getArenaTitle() + " :: " + QString("%1").arg(EISKALTDCPP_WND_TITLE));

    wcontainer->mapped(awgt);

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    ArenaWidget::Role role = awgt->role();

    bool widgetWithFilter = role == ArenaWidget::Hub ||
                            role == ArenaWidget::ShareBrowser ||
                            role == ArenaWidget::PublicHubs ||
                            role == ArenaWidget::Search ||
                            role == ArenaWidget::PrivateMessage;

    chatClear->setEnabled(role == ArenaWidget::Hub || role == ArenaWidget::PrivateMessage);
    findInWidget->setEnabled(widgetWithFilter);
    chatDisable->setEnabled(role == ArenaWidget::Hub);

    awgt->requestFocus();
}

void MainLayout::remWidgetFromArena(ArenaWidget *awgt){
    if (!arenaWidgets.contains(awgt))
        return;

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);

    if (arena->widget() == awgt->getWidget())
        setWindowTitle(QString("%1").arg(EISKALTDCPP_WND_TITLE));
}

void MainLayout::addArenaWidgetOnToolbar(ArenaWidget *awgt, bool keepFocus){
    if (!arenaWidgets.contains(awgt))
        return;

    QAction *act = new QAction(awgt->getArenaShortTitle(), this);
    act->setIcon(awgt->getPixmap());

    connect(act, SIGNAL(triggered()), this, SLOT(slotWidgetsToggle()));

    menuWidgetsActions.push_back(act);
    menuWidgetsHash.insert(act, awgt);

    menuWidgets->clear();
    menuWidgets->addActions(menuWidgetsActions);

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    emit addNewTab(awgt->getArenaShortTitle(), awgt->getWidget());
    wcontainer->insertWidget(awgt);
}

void MainLayout::remArenaWidgetFromToolbar(ArenaWidget *awgt){
    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
    for (; it != menuWidgetsHash.end(); ++it){
        if (it.value() == awgt){
            menuWidgetsActions.removeAt(menuWidgetsActions.indexOf(it.key()));
            menuWidgetsHash.erase(it);

            menuWidgets->clear();

            menuWidgets->addActions(menuWidgetsActions);

            break;
        }
    }

    if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);

    wcontainer->removeWidget(awgt);
    emit removeTab(awgt->getWidget());
}

void MainLayout::addActionOnToolBar(QAction *new_act){
    if (!fBar || toolBarActions.contains(new_act))
        return;

    fBar->insertAction(toolBarActions.last(), new_act);
    toolBarActions.append(new_act);
}

void MainLayout::remActionFromToolBar(QAction *act){
    if (!fBar || !toolBarActions.contains(act))
        return;

    fBar->removeAction(act);
    toolBarActions.removeAt(toolBarActions.indexOf(act));
}

void MainLayout::toggleSingletonWidget(ArenaWidget *a){
    if (!a)
        return;

    if (sender() && qobject_cast<QAction*>(sender()) && a->getWidget()){
        QAction *act = reinterpret_cast<QAction*>(sender());;

        act->setCheckable(true);

        a->setToolButton(act);
    }

    if (wcontainer->hasWidget(a)){
        QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.begin();
        for (; it != menuWidgetsHash.end(); ++it){
            if (it.value() == a){
                menuWidgetsActions.removeAt(menuWidgetsActions.indexOf(it.key()));
                menuWidgetsHash.erase(it);

                menuWidgets->clear();

                menuWidgets->addActions(menuWidgetsActions);

                break;
            }
        }

        emit removeTab(a->getWidget());

        wcontainer->removeWidget(a);
    }
    else {
        QAction *act = new QAction(a->getArenaShortTitle(), this);
        act->setIcon(a->getPixmap());

        connect(act, SIGNAL(triggered()), this, SLOT(slotWidgetsToggle()));

        menuWidgetsActions.push_back(act);
        menuWidgetsHash.insert(act, a);

        menuWidgets->clear();
        menuWidgets->addActions(menuWidgetsActions);

        emit addNewTab(a->getArenaShortTitle(), a->getWidget());

        wcontainer->insertWidget(a);
        wcontainer->mapped(a);
    }
}

void MainLayout::toggleMainMenu(bool showMenu){
    static QAction *compactMenus = NULL;

    menuBar()->setVisible(showMenu);

    if (showMenu){
        if (compactMenus && fBar)
            fBar->removeAction(compactMenus);
    }
    else {
        if (fBar){
            if (!compactMenus){
                compactMenus = new QAction(tr("Menu"), this);
                compactMenus->setObjectName("compactMenus");
                compactMenus->setIcon(WICON(WulforUtil::eiEDIT));
            }
            else {
                compactMenus->menu()->deleteLater();
                compactMenus->setMenu(NULL);
            }

            QMenu *m = new QMenu(this);

            foreach (QAction *a, menuBar()->actions())
                m->addAction(a);

            compactMenus->setMenu(m);

            connect(compactMenus, SIGNAL(triggered()), this, SLOT(slotShowMainMenu()));
        }

        if (fBar)
            fBar->insertAction(toolBarActions.first(), compactMenus);
    }

    WBSET(WB_MAIN_MENU_VISIBLE, showMenu);
}

void MainLayout::startSocket(bool changed){
    if (changed)
        ConnectivityManager::getInstance()->updateLast();
    try {
        ConnectivityManager::getInstance()->setup(true);
    } catch (const Exception& e) {
        showPortsError(e.getError());
    }
    ClientManager::getInstance()->infoUpdated();
}
void MainLayout::showPortsError(const string& port) {
    QString msg = tr("Unable to open %1 port. Searching or file transfers will not work correctly until you change settings or turn off any application that might be using that port.").arg(_q(port));
    QMessageBox::warning(this, tr("Connectivity Manager: Warning"), msg, QMessageBox::Ok);
}
void MainLayout::showShareBrowser(dcpp::UserPtr usr, const QString &file, const QString &jump_to){
    new ShareBrowser(usr, file, jump_to);
}

void MainLayout::reloadSomeSettings(){
    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }

    toolsSwitchSpeedLimit->setChecked(BOOLSETTING(THROTTLE_ENABLE));
}

void MainLayout::slotFileOpenLogFile(){
    QString f = QFileDialog::getOpenFileName(this, tr("Open log file"),_q(SETTING(LOG_DIRECTORY)), tr("Log files (*.log);;All files (*.*)"));

    if (!f.isEmpty()){
        f = QDir::toNativeSeparators(f);

        if (f.startsWith("/"))
            f = "file://" + f;
        else
            f = "file:///" + f;

        QDesktopServices::openUrl(f);
    }
}

void MainLayout::slotFileOpenDownloadDirectory(){
    QString directory = QString::fromStdString(SETTING(DOWNLOAD_DIRECTORY));

    directory.prepend( directory.startsWith("/")? ("file://") : ("file:///"));

    QDesktopServices::openUrl(QUrl::fromEncoded(directory.toUtf8()));
}

void MainLayout::slotFileBrowseOwnFilelist(){
    UserPtr user = ClientManager::getInstance()->getMe();
    QString file = QString::fromStdString(ShareManager::getInstance()->getOwnListFile());

    new ShareBrowser(user, file, "");
}

void MainLayout::slotFileHashProgress(){
    progress_dialog()->slotAutoClose(false);
    progress_dialog()->show();
}

void MainLayout::slotFileHasher(){
    FileHasher *m = new FileHasher(MainLayout::getInstance());
    m->setModal(true);
    m->exec();
    delete m;
    //FileHasher->show();
}

void MainLayout::slotFileRefreshShareHashProgress(){
    switch( HashProgress::getHashStatus() ) {
    case HashProgress::IDLE:
    {
        ShareManager *SM = ShareManager::getInstance();

        SM->setDirty();
        SM->refresh(true);

        updateHashProgressStatus();
        progress_dialog()->resetProgress();
    }
        break;
    case HashProgress::LISTUPDATE:
    case HashProgress::PAUSED:
    case HashProgress::DELAYED:
    case HashProgress::RUNNING:
        slotFileHashProgress();
        break;
    default:
        break;
    }
}

void MainLayout::slotOpenMagnet(){
    QString text = qApp->clipboard()->text(QClipboard::Clipboard);
    bool ok = false;

    text = (text.startsWith("magnet:?")? text : "");

    QString result = QInputDialog::getText(this, tr("Open magnet link"), tr("Enter magnet link:"), QLineEdit::Normal, text, &ok);

    if (!ok)
        return;

    if (result.startsWith("magnet:?")){
        Magnet m(this);

        m.setLink(result);

        m.exec();
    }
}

void MainLayout::slotHubsReconnect(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->reconnect();
}

void MainLayout::slotToolsADLS(){
    if (!ADLS::getInstance())
        ADLS::newInstance();

    toggleSingletonWidget(ADLS::getInstance());
}

void MainLayout::slotToolsSearch(){
    SearchFrame *sf = new SearchFrame();

    QLineEdit *le = qobject_cast<QLineEdit *>(sender());

    if (le == searchLineEdit){
        QString text = searchLineEdit->text();
        bool isTTH = false;

        if (!text.isEmpty()){
            if (text.startsWith("magnet:")){
                QString link = text;
                QString tth = "", name = "";
                int64_t size = 0;

                WulforUtil::splitMagnet(link, size, tth, name);

                text  = tth;
                isTTH = true;
            }
            else if (text.length() == 39){
                if (text.contains(QRegExp("[A-Z0-9]",Qt::CaseSensitive)))
                    isTTH = true;
            }
        }

        sf->fastSearch(text, isTTH);
    }
}

void MainLayout::slotToolsDownloadQueue(){
    if (!DownloadQueue::getInstance())
        DownloadQueue::newInstance();

    toggleSingletonWidget(DownloadQueue::getInstance());
}

void MainLayout::slotToolsQueuedUsers(){
    if (!QueuedUsers::getInstance())
        QueuedUsers::newInstance();

    toggleSingletonWidget(QueuedUsers::getInstance());
}

void MainLayout::slotToolsHubManager(){
    if (!HubManager::getInstance())
        HubManager::newInstance();

    toggleSingletonWidget(HubManager::getInstance());
}

void MainLayout::slotToolsFinishedDownloads(){
    if (!FinishedDownloads::getInstance())
        FinishedDownloads::newInstance();

    toggleSingletonWidget(FinishedDownloads::getInstance());
}

void MainLayout::slotToolsFinishedUploads(){
   if (!FinishedUploads::getInstance())
        FinishedUploads::newInstance();

    toggleSingletonWidget(FinishedUploads::getInstance());
}

void MainLayout::slotToolsSpy(){
    if (!SpyFrame::getInstance())
        SpyFrame::newInstance();

    toggleSingletonWidget(SpyFrame::getInstance());
}

void MainLayout::slotToolsAntiSpam(){
    AntiSpamFrame fr(this);

    fr.exec();

    toolsAntiSpam->setChecked(AntiSpam::getInstance() != NULL);
}

void MainLayout::slotToolsIPFilter(){
    IPFilterFrame fr(this);

    fr.exec();

    toolsIPFilter->setChecked(IPFilter::getInstance() != NULL);
}

void MainLayout::slotToolsAutoAway(){
    WBSET(WB_APP_AUTO_AWAY, toolsAutoAway->isChecked());
}

void MainLayout::slotToolsSwitchAway(){
    //qDebug() << sender();

    if ((sender() != toolsAwayOff) && (sender() != toolsAwayOn))
        return;

    bool away = toolsAwayOn->isChecked();

    Util::setAway(away);
    Util::setManualAway(away);
}

void MainLayout::slotToolsJS(){
#ifdef USE_JS
    ScriptManagerDialog(this).exec();
#endif
}

void MainLayout::slotToolsJSConsole(){
#ifdef USE_JS
    ScriptConsole sc(this);
    sc.setWindowModality(Qt::NonModal);
    sc.exec();
#endif
}

void MainLayout::slotHubsFavoriteHubs(){
    if (!FavoriteHubs::getInstance())
        FavoriteHubs::newInstance();

    toggleSingletonWidget(FavoriteHubs::getInstance());
}

void MainLayout::slotHubsPublicHubs(){
    if (!PublicHubs::getInstance())
        PublicHubs::newInstance();

    toggleSingletonWidget(PublicHubs::getInstance());
}

void MainLayout::slotHubsFavoriteUsers(){
    if (!FavoriteUsers::getInstance())
        FavoriteUsers::newInstance();

    toggleSingletonWidget(FavoriteUsers::getInstance());
}

void MainLayout::slotToolsCopyWindowTitle(){
    QString text = windowTitle();

    if (!text.isEmpty())
        qApp->clipboard()->setText(text, QClipboard::Clipboard);
}

void MainLayout::slotToolsSettings(){
    Settings s;

    s.exec();

    reloadSomeSettings();

    //reload some settings
    if (!WBGET(WB_TRAY_ENABLED))
        fileHideWindow->setText(tr("Show/hide find frame"));
    else
        fileHideWindow->setText(tr("Hide window"));
}

void MainLayout::slotToolsTransfer(bool toggled){
    if (toggled){
        transfer_dock->setVisible(true);
        transfer_dock->setWidget(TransferView::getInstance());
    }
    else {
        transfer_dock->setWidget(NULL);
        transfer_dock->setVisible(false);
    }
}

void MainLayout::slotToolsSwitchSpeedLimit(){
    static WulforUtil *WU = WulforUtil::getInstance();

    SettingsManager::getInstance()->set(SettingsManager::THROTTLE_ENABLE, toolsSwitchSpeedLimit->isChecked());
    toolsSwitchSpeedLimit->setIcon(BOOLSETTING(THROTTLE_ENABLE)? WU->getPixmap(WulforUtil::eiSPEED_LIMIT_ON) : WU->getPixmap(WulforUtil::eiSPEED_LIMIT_OFF));
}

void MainLayout::slotPanelMenuActionClicked(){
    QAction *act = qobject_cast<QAction *>(sender());

    if (act == 0)
        return;

    if (act == panelsWidgets){
        if (tBar)
            tBar->setVisible(panelsWidgets->isChecked());
        else if (mBar)
            mBar->setVisible(panelsWidgets->isChecked());
        else if (sideDock)
            sideDock->setVisible(panelsWidgets->isChecked());
        WBSET(WB_WIDGETS_PANEL_VISIBLE, panelsWidgets->isChecked());
    }
    else if (act == panelsTools){
        fBar->setVisible(panelsTools->isChecked());
        WBSET(WB_TOOLS_PANEL_VISIBLE, panelsTools->isChecked());
    }
    else if (act == panelsSearch){
        sBar->setVisible(panelsSearch->isChecked());
        WBSET(WB_SEARCH_PANEL_VISIBLE, panelsSearch->isChecked());
    }
}

void MainLayout::slotChatClear(){
    HubFrame *fr = qobject_cast<HubFrame *>(arena->widget());
    PMWindow *pm = qobject_cast<PMWindow *>(arena->widget());

    if (fr)
        fr->clearChat();
    else if (pm)
        pm->clearChat();
}

void MainLayout::slotFind(){
    if (!arena->widget() || !qobject_cast<ArenaWidget*>(arena->widget()))
        return;

    ArenaWidget *awgt = qobject_cast<ArenaWidget*>(arena->widget());
    awgt->requestFilter();
}

void MainLayout::slotChatDisable(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->disableChat();
}

void MainLayout::slotWidgetsToggle(){
    QAction *act = reinterpret_cast<QAction*>(sender());
    QHash<QAction*, ArenaWidget*>::iterator it = menuWidgetsHash.find(act);

    if (it == menuWidgetsHash.end())
        return;

    mapWidgetOnArena(it.value());
}

void MainLayout::slotQC(){
    QuickConnect qc;

    qc.exec();
}

void MainLayout::slotHideMainMenu(){
    toggleMainMenu(!menuBar()->isVisible());
}

void MainLayout::slotShowMainMenu() {
    QAction *act = qobject_cast<QAction*>(sender());

    if (!(act && act->menu()))
        return;

    act->menu()->exec(QCursor::pos());
}

void MainLayout::slotHideWindow(){
    if (!isUnload && isActiveWindow() && WBGET(WB_TRAY_ENABLED)) {
        hide();
    }
}

void MainLayout::slotHideProgressSpace() {
    if (WBGET(WB_SHOW_FREE_SPACE)) {
        progressSpace->hide();
        toolsHideProgressSpace->setText(tr("Show free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, false);
    } else {
        progressSpace->show();
        toolsHideProgressSpace->setText(tr("Hide free space bar"));

        WBSET(WB_SHOW_FREE_SPACE, true);
    }
}

void MainLayout::slotHideLastStatus(){
    bool st = WBGET(WB_LAST_STATUS);

    st = !st;

    if (!st)
        toolsHideLastStatus->setText(tr("Show last status message"));
    else
        toolsHideLastStatus->setText(tr("Hide last status message"));

    WBSET(WB_LAST_STATUS, st);

    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }
}

void MainLayout::slotHideUsersStatistics(){
    bool st = WBGET(WB_USERS_STATISTICS);

    st = !st;

    if (!st)
        toolsHideUsersStatisctics->setText(tr("Show users statistics"));
    else
        toolsHideUsersStatisctics->setText(tr("Hide users statistics"));

    WBSET(WB_USERS_STATISTICS, st);

    for (int k = 0; k < arenaWidgets.size(); ++k){
        HubFrame *fr = qobject_cast<HubFrame *>(arenaMap[arenaWidgets.at(k)]);

        if (fr)
            fr->reloadSomeSettings();
    }
}

void MainLayout::slotExit(){
    setUnload(true);

    close();
}

void MainLayout::slotToolbarCustomization() {
    QMenu *m = new QMenu(this);

    QMenu *toolButtonStyle = new QMenu(tr("Button style"), this);
    toolButtonStyle->addAction(tr("Icons only"))->setData(Qt::ToolButtonIconOnly);
    toolButtonStyle->addAction(tr("Text only"))->setData(Qt::ToolButtonTextOnly);
    toolButtonStyle->addAction(tr("Text beside icons"))->setData(Qt::ToolButtonTextBesideIcon);
    toolButtonStyle->addAction(tr("Text under icons"))->setData(Qt::ToolButtonTextUnderIcon);

    foreach (QAction *a, toolButtonStyle->actions()){
        a->setCheckable(true);
        a->setChecked(fBar->toolButtonStyle() == static_cast<Qt::ToolButtonStyle>(a->data().toInt()));
    }

    m->addMenu(toolButtonStyle);
    m->addSeparator();

    QAction *customize = m->addAction(tr("Customize"));
    QAction *ret = m->exec(QCursor::pos());

    m->deleteLater();
    toolButtonStyle->deleteLater();

    if (ret == customize){
        ActionCustomizer customizer(toolBarActions, fBar->actions(), this);
        connect(&customizer, SIGNAL(done(QList<QAction*>)), this, SLOT(slotToolbarCustomizerDone(QList<QAction*>)));

        customizer.exec();
    }
    else if (ret){
        fBar->setToolButtonStyle(static_cast<Qt::ToolButtonStyle>(ret->data().toInt()));

        WISET(TOOLBUTTON_STYLE, static_cast<int>(fBar->toolButtonStyle()));
    }
}

void MainLayout::slotToolbarCustomizerDone(const QList<QAction*> &enabled){
    fBar->clear();
    QStringList enabled_list;

    foreach (QAction *act, enabled){
        if (!act)
            continue;

        fBar->addAction(act);
        enabled_list.push_back(act->objectName());
    }

    WSSET(WS_MAINWINDOW_TOOLBAR_ACTS, enabled_list.join(";").toAscii().toBase64());
}

void MainLayout::slotAboutOpenUrl(){
    QAction *act = qobject_cast<QAction *>(sender());
    if (act == aboutHomepage){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/"));
    }
    else if (act == aboutSource){
        QDesktopServices::openUrl(QUrl("http://github.com/negativ/eiskaltdcpp/"));
    }
    else if (act == aboutIssues){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/issues/list"));
    }
    else if (act == aboutWiki){
        QDesktopServices::openUrl(QUrl("http://code.google.com/p/eiskaltdc/w/list"));
    }
    else if (act == aboutChangelog){
        // Now available: ChangeLog.txt, ChangeLog_ru.txt, ChangeLog_uk.txt
        QDesktopServices::openUrl(QUrl(tr("http://github.com/negativ/eiskaltdcpp/raw/master/ChangeLog.txt")));
    }
}

void MainLayout::slotAboutClient(){
    About a(this);

    volatile const qulonglong &app_total_down = totalDown;
    volatile const qulonglong &app_total_up   = totalUp;

    a.label->setText(QString("<b>%1</b> %2 (%3)")
                     .arg(EISKALTDCPP_WND_TITLE)
                     .arg(EISKALTDCPP_VERSION)
                     .arg(EISKALTDCPP_VERSION_SFX));

    a.label_ABOUT->setTextFormat(Qt::RichText);

    QString about_text = tr("EiskaltDC++ is a graphical client for Direct Connect and ADC protocols.<br/><br/>"
                            ""
                            "DC++ core version: %1 (modified)<br/><br/>"
                            ""
                            "Home page: <a href=\"http://code.google.com/p/eiskaltdc/\">http://code.google.com/p/eiskaltdc/</a><br/><br/>"
                            ""
                            "Total up: <b>%2</b><br/>"
                            "Total down: <b>%3</b><br/>"
                            "Ratio: <b>%4</b>"
                         ).arg(DCVERSIONSTRING)
                          .arg(WulforUtil::formatBytes(app_total_up))
                          .arg(WulforUtil::formatBytes(app_total_down))
                          .arg((float)app_total_up/(float)app_total_down, 0, 'f', 2);

    a.label_ABOUT->setText(about_text);

    a.textBrowser_AUTHORS->setText(
            tr("Please use <a href=\"http://code.google.com/p/eiskaltdc/issues/list\">"
               "http://code.google.com/p/eiskaltdc/issues/list</a> to report bugs.<br/>")+
            tr("<br/>"
               "<b>Developers</b><br/>")+
            tr("<br/>"
               "&nbsp;Andrey Karlov<br/>"
               "&nbsp;&lt;dein.negativ@gmail.com&gt;<br/>"
               "&nbsp;(main developer 0.4.10 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Boris Pek  aka  Tehnick<br/>"
               "&nbsp;&lt;tehnick-8@mail.ru&gt;<br/>"
               "&nbsp;(maintainer and developer 1.89.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Eugene Petrov<br/>"
               "&nbsp;&lt;dhamp@ya.ru&gt;<br/>"
               "&nbsp;(maintainer and developer 0.4.10 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Alexandr Tkachev<br/>"
               "&nbsp;&lt;tka4ev@gmail.com&gt;<br/>"
               "&nbsp;(developer 2.0.3 and later)<br/>")+
            tr("<br/>"
               "<b>Logo and Splash Screen Logo</b><br/>")+
            tr("<br/>"
               "&nbsp;Uladzimir Bely<br/>"
               "&nbsp;&lt;wiselord1983@gmail.com&gt;<br/>"
               "&nbsp;(version 0.4.10 and later)<br/>")
            );

    a.textBrowser_TRANSLATION->setText(
            tr("<b>Translators</b><br/>")+
            tr("<br/>"
               "&nbsp;<u>Russian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Uladzimir Bely<br/>"
               "&nbsp;&lt;wiselord1983@gmail.com&gt;<br/>"
               "&nbsp;(for 0.4.10 and later)<br/>")+
            tr("<br/>"
               "&nbsp;Boris Pek  aka  Tehnick<br/>"
               "&nbsp;&lt;tehnick-8@mail.ru&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Belarusian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Paval Shalamitski  aka  Klyok<br/>"
               "&nbsp;&lt;i.kliok@gmail.com&gt;<br/>"
               "&nbsp;(for 1.0.40 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Hungarian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Akos Berki  aka  sumo<br/>"
               "&nbsp;&lt;husumo@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.1 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>French translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Alexandre Wallimann  aka  Ale<br/>"
               "&nbsp;&lt;alexandre.wallimann@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.2 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Polish translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Arahael<br/>"
               "&nbsp;(for 2.0.2 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Ukrainian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Dmytro Demenko<br/>"
               "&nbsp;&lt;dmytro.demenko@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.3 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Serbian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Miroslav Petrovic<br/>"
               "&nbsp;&lt;miroslav031@gmail.com&gt;<br/>"
               "&nbsp;(for 2.0.3 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Spanish translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Gustavo Alvarez aka sL1pKn07<br/>"
               "&nbsp;&lt;sl1pkn07@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Bulgarian translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Rusi Dimitrov aka PsyTrip<br/>"
               "&nbsp;&lt;dimitrov.rusi@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Slovak translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Martin Durisin<br/>"
               "&nbsp;&lt;martin.durisin@gmail.com&gt;<br/>"
               "&nbsp;(for 2.1.0 and later)<br/>")+
            tr("<br/>"
               "&nbsp;<u>Czech translation</u><br/>")+
            tr("<br/>"
               "&nbsp;Uhlik<br/>"
               "&nbsp;&lt;uhlikx@seznam.cz&gt;<br/>"
               "&nbsp;(for 2.2.0 and later)<br/>")
            );

    a.exec();
}

void MainLayout::slotUnixSignal(int sig){
    printf("Received unix signal %i\n", sig);
}

void MainLayout::slotCloseCurrentWidget(){
    if (arena->widget())
        arena->widget()->close();
}

void MainLayout::slotSidebarContextMenu(){
    QItemSelectionModel *s_m =sideTree->selectionModel();
    QModelIndexList selected = s_m->selectedRows(0);

    if (selected.size() < 1)
        return;

    SideBarItem *item = reinterpret_cast<SideBarItem*>(selected.at(0).internalPointer());

    QMenu *menu = NULL;
    if (item && item->childCount() > 0){
        menu = new QMenu(this);
        menu->addAction(WICON(WulforUtil::eiEDITDELETE), tr("Close all"));

        if (menu->exec(QCursor::pos())){
            QList<SideBarItem*> childs = item->childItems;

            foreach (SideBarItem *i, childs){
                if (i && i->getWidget())
                    i->getWidget()->getWidget()->close();
            }
        }

        menu->deleteLater();

        return;
    }
    else if (item && item->getWidget()){
        menu = item->getWidget()->getMenu();

        if(!menu){
            menu = new QMenu(this);
            menu->addAction(WICON(WulforUtil::eiEDITDELETE), tr("Close"));

            if (menu->exec(QCursor::pos()))
                item->getWidget()->getWidget()->close();

            menu->deleteLater();
        }
        else
            menu->exec(QCursor::pos());
    }
}

void MainLayout::slotSidebarHook(const QModelIndex &index){
    if (index.column() == 1){
        SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());

        if (item->getWidget()){
            switch (item->getWidget()->role()){
            case ArenaWidget::Hub:
            case ArenaWidget::PrivateMessage:
            case ArenaWidget::Search:
            case ArenaWidget::ShareBrowser:
            case ArenaWidget::CustomWidget:
                item->getWidget()->getWidget()->close();
                break;
            default:
                break;
            }
        }
    }
}

void MainLayout::slotSideBarDblClicked(const QModelIndex &index){
    if (index.column() != 0)
        return;
    SideBarModel *model = reinterpret_cast<SideBarModel*>(sideTree->model());
    SideBarItem *item = reinterpret_cast<SideBarItem*>(index.internalPointer());

    if (!model->isRootItem(item) || item->childCount() > 0)
        return;

    switch (model->rootItemRole(item)){
    case ArenaWidget::Search:
        {
            slotToolsSearch();

            break;
        }
    case ArenaWidget::Hub:
        {
            slotQC();

            break;
        }
    case ArenaWidget::ShareBrowser:
        {
            slotFileBrowseFilelist();

            break;
        }
    case ArenaWidget::PrivateMessage:
        {
            slotFileOpenLogFile();

            break;
        }
    default:
        break;
    }

    sideTree->setExpanded(index, true);
}

void MainLayout::slotSideBarDockMenu(){
    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(WBGET(SIDEBAR_SHOW_CLOSEBUTTONS, true));

    m->addAction(act);

    if (m->exec(QCursor::pos()) != NULL){
        WBSET(SIDEBAR_SHOW_CLOSEBUTTONS, act->isChecked());

        //repaint rows!
        if (sideDock && act->isChecked())
            sideDock->resize(sideDock->size()+QSize(18, 0));
        else if(sideDock)
            sideDock->resize(sideDock->size()+QSize(-18, 0));
    }

    m->deleteLater();
}

void MainLayout::slotSelectSidebarIndex(const QModelIndex &index){
    QItemSelectionModel *s_m =sideTree->selectionModel();

    s_m->select(index, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
}

void MainLayout::slotAboutQt(){
    QMessageBox::aboutQt(this);
}

void MainLayout::nextMsg(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->nextMsg();
    else{
        QWidget *wg = arena->widget();

        bool pmw = false;

        if (wg != 0)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->nextMsg();
        }
    }
}

void MainLayout::prevMsg(){
    HubFrame *fr = HubManager::getInstance()->activeHub();

    if (fr)
        fr->prevMsg();
    else{
        QWidget *wg = arena->widget();

        bool pmw = false;

        if (wg != 0)
            pmw = (typeid(*wg) == typeid(PMWindow));

        if(pmw){
            PMWindow *pm = qobject_cast<PMWindow *>(wg);

            if (pm)
                pm->prevMsg();
        }
    }
}

void MainLayout::on(dcpp::LogManagerListener::Message, time_t t, const std::string& m) throw(){
    emit coreLogMessage(_q(m.c_str()));
}

void MainLayout::on(dcpp::QueueManagerListener::Finished, QueueItem *item, const std::string &dir, int64_t) throw(){
    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST)){
        UserPtr user = item->getDownloads()[0]->getUser();
        QString listName = _q(item->getListName());

        emit coreOpenShare(user, listName, _q(dir));
    }

    const int qsize = QueueManager::getInstance()->lockQueue().size();
    QueueManager::getInstance()->unlockQueue();

    if (qsize == 1)
        emit notifyMessage(Notification::TRANSFER, tr("Download Queue"), tr("All downloads complete"));
}

void MainLayout::on(dcpp::TimerManagerListener::Second, uint64_t ticks) throw(){
    static quint32 lastUpdate = 0;
    static quint64 lastUp = 0, lastDown = 0;

    quint64 now = GET_TICK();

    quint64 diff = now - lastUpdate;
    quint64 downBytes = 0;
    quint64 upBytes = 0;

    if (diff < 100U)
        diff = 1U;

    quint64 downDiff = Socket::getTotalDown() - lastDown;
    quint64 upDiff = Socket::getTotalUp() - lastUp;

    downBytes = (downDiff * 1000) / diff;
    upBytes = (upDiff * 1000) / diff;

    QMap<QString, QString> map;

    map["STATS"]    = _q(Client::getCounts());
    map["DSPEED"]   = WulforUtil::formatBytes(downBytes);
    map["DOWN"]     = WulforUtil::formatBytes(Socket::getTotalDown());
    map["USPEED"]   = WulforUtil::formatBytes(upBytes);
    map["UP"]       = WulforUtil::formatBytes(Socket::getTotalUp());

    totalDown = totalDown+downDiff;
    totalUp = totalUp+upDiff;
    lastUpdate = ticks;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

    emit coreUpdateStats(map);
}


LC_EXPORT_PLUGIN (leechcraft_eiskaltdcpp, MainLayout);
